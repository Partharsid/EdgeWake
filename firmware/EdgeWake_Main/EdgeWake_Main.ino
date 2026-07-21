/*
 * ============================================================
 *  EdgeWake — Simplified Forest Guardian
 * ============================================================
 *  This code protects forests using 5 "Tiers" to save battery.
 *  It only does work when it absolutely has to.
 *  
 *  Read the comments below to understand how it works!
 */

#include "config.h"
#include "camera_utils.h"
#include "audio_utils.h"
#include "telegram.h"       // Replaced the old network_utils.h
#include "esp_sleep.h"
#include "driver/rtc_io.h"

// Track how many times the board has woken up.
// RTC_DATA_ATTR means this number survives "Deep Sleep".
RTC_DATA_ATTR int bootCount = 0;

void setup() {
  // Start the serial monitor so we can read messages on the computer
  Serial.begin(115200);
  delay(1000); // Wait a second for it to connect
  
  bootCount++;
  Serial.println("\n\n==========================================");
  Serial.println("🌲 EdgeWake — Forest Guardian Started! 🌲");
  Serial.print("Boot number: ");
  Serial.println(bootCount);
  Serial.println("==========================================");

  // -------------------------------------------------------------
  // TIER 1 & TIER 2: Check why the board woke up
  // -------------------------------------------------------------
  // The board was in "Deep Sleep" (Tier 1). Let's see if a sensor 
  // woke it up (Tier 2), or if you just plugged it in for the first time.
  
  esp_sleep_wakeup_cause_t wakeReason = esp_sleep_get_wakeup_cause();
  
  if (wakeReason != ESP_SLEEP_WAKEUP_EXT1) {
    // If it wasn't woken up by our sensors (EXT1), it means it's a cold boot.
    // We don't need to do anything, just go to sleep and wait for a real threat!
    Serial.println("Just turned on. Nothing to report.");
    goToDeepSleep();
    return;
  }

  // If we are here, a sensor was triggered! Let's find out which one.
  uint64_t wakeupPins = esp_sleep_get_ext1_wakeup_status();
  String alertType = "Unknown";
  
  // Check if it was the Flame sensor
  if (wakeupPins & (1ULL << FLAME_SENSOR_PIN)) {
    Serial.println("🔥 FLAME SENSOR TRIGGERED!");
    alertType = "Fire";
  }
  // Check if it was the Vibration sensor
  else if (wakeupPins & (1ULL << VIBRATION_SENSOR_PIN)) {
    Serial.println("📳 VIBRATION SENSOR TRIGGERED!");
    alertType = "Vibration";
  }

  // -------------------------------------------------------------
  // TIER 3: Edge Verification (Listen with the Microphone & AI)
  // -------------------------------------------------------------
  // We don't want to send false alarms (like a branch falling).
  // Let's turn on the microphone and ask the AI if it sounds like danger.
  
  bool isRealThreat = false;
  
  if (initMicrophone()) {
    isRealThreat = verifyThreatAudio(); // Asks the AI
    deinitMicrophone(); // Turn off microphone to save power and free up the pins for the camera
  } else {
    Serial.println("Microphone failed! Assuming it's a real threat just to be safe.");
    isRealThreat = true;
  }
  
  if (!isRealThreat) {
    // AI says it's a false alarm!
    Serial.println("AI says it's a FALSE ALARM. Going back to sleep.");
    goToDeepSleep();
    return;
  }

  // -------------------------------------------------------------
  // TIER 4: Visual Capture (Take a Photo)
  // -------------------------------------------------------------
  // AI confirmed the threat! Now we need picture evidence.
  
  camera_fb_t *photo = NULL;
  
  if (initCamera()) {
    photo = capturePhoto();
  } else {
    Serial.println("Camera failed to start!");
  }
  
  // -------------------------------------------------------------
  // TIER 5: Cloud Handoff (Send to Telegram)
  // -------------------------------------------------------------
  // We have evidence. Turn on WiFi and send a message directly to your phone.
  
  if (photo) {
    String message = "🚨 EDGEWAKE ALERT 🚨\n";
    message += "Danger Detected: " + alertType + "\n";
    message += "The AI verified this threat. See attached photo.";
    
    bool sent = sendAlertToTelegram(photo, message);
    
    if (sent) {
      Serial.println("✅ Alert sent to your phone!");
    } else {
      Serial.println("❌ Failed to send alert.");
    }
    
    // Free the camera memory now that we sent it
    esp_camera_fb_return(photo);
  } else {
    Serial.println("No photo to send! Skipping Telegram.");
  }

  // -------------------------------------------------------------
  // DONE: Go back to Deep Sleep (Back to Tier 1)
  // -------------------------------------------------------------
  goToDeepSleep();
}

void loop() {
  // This function is never used because the board completely 
  // turns off during Deep Sleep, and restarts at setup() when it wakes up.
}

// -------------------------------------------------------------
// HELPER FUNCTION: Go to Deep Sleep
// -------------------------------------------------------------
void goToDeepSleep() {
  Serial.println("Configuring Deep Sleep...");

  // Tell the ESP32 to wake up if ANY of our sensor pins send a HIGH signal
  uint64_t wakeMask = (1ULL << FLAME_SENSOR_PIN) | (1ULL << VIBRATION_SENSOR_PIN);
  esp_sleep_enable_ext1_wakeup(wakeMask, ESP_EXT1_WAKEUP_ANY_HIGH);

  // Isolate the microphone pin so it doesn't drain battery while sleeping
  rtc_gpio_isolate((gpio_num_t)I2S_SD_PIN);

  Serial.println("Sleeping for 5 seconds first so the sensors can settle down...");
  delay(5000); // Simple fix! This prevents the sensor from immediately waking us up again.

  Serial.println("Entering Deep Sleep... Zzz... 🌙");
  Serial.flush(); 
  
  // Turn off the brain!
  esp_deep_sleep_start();
}

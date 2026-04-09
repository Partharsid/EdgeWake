/*
 * ============================================================
 *  EdgeWake — Tiered Cascade Guardian
 *  Main Firmware for ESP32-CAM (AI-Thinker)
 * ============================================================
 *
 *  Architecture:
 *    Tier 1 — Deep Sleep  (near-zero power)
 *    Tier 2 — Hardware Interrupt  (flame/vibration → ext1 wake)
 *    Tier 3 — Edge Verification   (INMP441 audio analysis)
 *    Tier 4 — Visual Capture      (OV2640 JPEG snapshot)
 *    Tier 5 — Cloud Handoff       (Wi-Fi → n8n webhook → sleep)
 *
 *  Wake sources (ext1 — multi-pin):
 *    GPIO 13  →  Flame / Smoke Sensor
 *    GPIO 2   →  SW-420 Vibration Sensor
 *
 *  Board:  "AI Thinker ESP32-CAM"
 *  Upload: Via FTDI — connect IO0 to GND during upload,
 *          then remove jumper & reset to run.
 *
 *  Required Libraries (install via Arduino Library Manager):
 *    - ESP32 board package (by Espressif)
 *
 *  No external libraries needed — everything uses
 *  ESP-IDF drivers bundled with the ESP32 Arduino core.
 * ============================================================
 */

#include "config.h"
#include "camera_utils.h"
#include "audio_utils.h"
#include "network_utils.h"

#include "esp_sleep.h"
#include "esp_wifi.h"
#include "driver/rtc_io.h"

// ─── Boot counter (persists across deep-sleep cycles) ───
RTC_DATA_ATTR int bootCount = 0;
RTC_DATA_ATTR unsigned long lastTriggerMs = 0;

// ─── Forward declarations ───────────────────────────────
void enterDeepSleep();
void printWakeupReason();
const char* identifyTriggerSource();

// =========================================================
//  setup()  — runs once on EVERY wake-up (cold boot + wake)
// =========================================================
void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(200);  // let serial settle

  bootCount++;
  Serial.println();
  Serial.println("╔══════════════════════════════════════════╗");
  Serial.println("║      🌲  EdgeWake — Forest Guard  🌲     ║");
  Serial.println("╠══════════════════════════════════════════╣");
  Serial.printf( "║  Boot #%d  |  Device: %-17s  ║\n", bootCount, DEVICE_ID);
  Serial.println("╚══════════════════════════════════════════╝");

  printWakeupReason();

  // ── Determine why we woke up ──────────────────────────
  esp_sleep_wakeup_cause_t wakeReason = esp_sleep_get_wakeup_cause();

  if (wakeReason != ESP_SLEEP_WAKEUP_EXT1) {
    // First boot or unexpected wake — just go to sleep.
    Serial.println("[MAIN] Cold boot or non-trigger wake. Going to sleep...");
    enterDeepSleep();
    return;  // never reached
  }

  // ═════════════════════════════════════════════════════
  //  TIER 2 — Hardware Interrupt Triggered!
  //  Identify WHICH sensor caused the wake-up.
  // ═════════════════════════════════════════════════════
  const char *alertType = identifyTriggerSource();

  Serial.printf("⚡ [TIER 2] HARDWARE INTERRUPT DETECTED! Source: %s\n", alertType);

  // De-bounce: ignore re-triggers too close together
  // (RTC memory persists across sleep cycles)
  if (lastTriggerMs > 0 && (millis() < DEBOUNCE_MS)) {
    Serial.println("[MAIN] Debounce active — ignoring this trigger.");
    enterDeepSleep();
    return;
  }

  // ═════════════════════════════════════════════════════
  //  TIER 3 — Edge Verification (Audio Analysis)
  // ═════════════════════════════════════════════════════
  Serial.println("🎙️ [TIER 3] Starting audio verification...");

  bool micOk = initMicrophone();
  bool threatConfirmed = false;

  if (micOk) {
    threatConfirmed = verifyThreatAudio();
    deinitMicrophone();  // free I2S resources before camera
  } else {
    Serial.println("[MAIN] Mic init failed — assuming threat (fail-open).");
    threatConfirmed = true;
  }

  if (!threatConfirmed) {
    Serial.println("[MAIN] Audio says false alarm. Back to sleep.");
    enterDeepSleep();
    return;
  }

  // ═════════════════════════════════════════════════════
  //  TIER 4 — Visual Capture
  // ═════════════════════════════════════════════════════
  Serial.println("📷 [TIER 4] Initialising camera for visual capture...");

  bool camOk = initCamera();
  camera_fb_t *photo = NULL;

  if (camOk) {
    photo = capturePhoto();
  } else {
    Serial.println("[MAIN] Camera init failed!");
  }

  // ═════════════════════════════════════════════════════
  //  TIER 5 — Cloud Handoff (Wi-Fi → n8n webhook)
  // ═════════════════════════════════════════════════════
  Serial.println("☁️ [TIER 5] Connecting to Wi-Fi for cloud handoff...");

  bool wifiOk = connectWiFi();

  if (wifiOk) {
    if (photo) {
      bool sent = sendAlert(photo, alertType, 0);
      if (sent) {
        Serial.println("[MAIN] ✅ Alert sent successfully!");
      } else {
        Serial.println("[MAIN] ❌ Alert send failed.");
      }
    } else {
      Serial.println("[MAIN] No photo available — sending text-only alert.");
      // Send a text-only fallback via JSON
      HTTPClient http;
      http.begin(WEBHOOK_URL);
      http.addHeader("Content-Type", "application/json");
      String json = "{\"alert_type\":\"" + String(alertType) + "\","
                    "\"device_id\":\"" + String(DEVICE_ID) + "\","
                    "\"location\":\"" + String(DEVICE_LOCATION) + "\","
                    "\"error\":\"camera_failed\","
                    "\"timestamp\":" + String(millis()) + "}";
      int code = http.POST(json);
      Serial.printf("[MAIN] Fallback alert response: %d\n", code);
      http.end();
    }
    disconnectWiFi();
  } else {
    Serial.println("[MAIN] Wi-Fi failed — alert NOT sent. Going back to sleep.");
  }

  // Return the camera frame buffer
  if (photo) {
    esp_camera_fb_return(photo);
  }

  // Record this trigger time for debounce
  lastTriggerMs = millis();

  // ═════════════════════════════════════════════════════
  //  Back to TIER 1 — Deep Sleep
  // ═════════════════════════════════════════════════════
  Serial.println("[MAIN] Full cycle complete. Returning to deep sleep...");
  enterDeepSleep();
}

// =========================================================
//  loop()  — never runs (deep-sleep resets the chip)
// =========================================================
void loop() {
  // This function intentionally left empty.
  // The ESP32 restarts from setup() on every deep-sleep wake.
}

// =========================================================
//  Identify which sensor caused the ext1 wake-up
// =========================================================
const char* identifyTriggerSource() {
  uint64_t wakeupBits = esp_sleep_get_ext1_wakeup_status();

  bool flameFired     = (wakeupBits & (1ULL << FLAME_SENSOR_PIN))     != 0;
  bool vibrationFired = (wakeupBits & (1ULL << VIBRATION_SENSOR_PIN)) != 0;

  if (flameFired && vibrationFired) {
    Serial.println("[TIER 2] Both FLAME + VIBRATION sensors triggered simultaneously.");
    return "Fire";  // prioritise fire
  } else if (flameFired) {
    Serial.println("[TIER 2] 🔥 Flame / smoke sensor triggered on GPIO 13.");
    return "Fire";
  } else if (vibrationFired) {
    Serial.println("[TIER 2] 📳 Vibration sensor triggered on GPIO 2.");
    Serial.println("[TIER 2] Abnormal vibration or sound detected!");
    return "Vibration";
  } else {
    Serial.printf("[TIER 2] Unknown trigger source (bits: 0x%llx)\n", wakeupBits);
    return "Unknown";
  }
}

// =========================================================
//  Helper: Configure ext1 wake-up and enter deep sleep
// =========================================================
void enterDeepSleep() {
  Serial.println("💤 [SLEEP] Configuring ext1 wake-up on GPIO 13 + GPIO 2...");

  // Isolate GPIO 12 (I2S SD pin) to prevent current leakage during sleep
  rtc_gpio_isolate(GPIO_NUM_12);

  // Configure ext1: wake when ANY pin in the bitmask goes HIGH
  esp_sleep_enable_ext1_wakeup(EXT1_WAKEUP_MASK, ESP_EXT1_WAKEUP_ANY_HIGH);

  Serial.println("💤 [SLEEP] Entering deep sleep... goodnight. 🌙");
  Serial.flush();
  delay(100);

  esp_deep_sleep_start();

  // ── Execution never reaches here ──
}

// =========================================================
//  Helper: Print human-readable wake-up reason
// =========================================================
void printWakeupReason() {
  esp_sleep_wakeup_cause_t reason = esp_sleep_get_wakeup_cause();

  switch (reason) {
    case ESP_SLEEP_WAKEUP_EXT0:
      Serial.println("[WAKE] Cause: EXT0 (Single Pin Interrupt)");
      break;
    case ESP_SLEEP_WAKEUP_EXT1:
      Serial.println("[WAKE] Cause: EXT1 (Multi-Sensor Interrupt)");
      break;
    case ESP_SLEEP_WAKEUP_TIMER:
      Serial.println("[WAKE] Cause: TIMER");
      break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD:
      Serial.println("[WAKE] Cause: TOUCHPAD");
      break;
    case ESP_SLEEP_WAKEUP_ULP:
      Serial.println("[WAKE] Cause: ULP co-processor");
      break;
    default:
      Serial.println("[WAKE] Cause: COLD BOOT (power-on / reset)");
      break;
  }
}

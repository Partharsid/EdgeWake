/*
 * ============================================================
 *  EdgeWake — Configuration Header
 *  Project : Forest Guard (Tiered Cascade Guardian)
 * ============================================================
 *
 *  !! IMPORTANT !!
 *  Edit only this file before uploading to your ESP32-CAM.
 *  Every tuneable parameter lives here.
 * ============================================================
 */

#ifndef CONFIG_H
#define CONFIG_H

// ─────────────────────────────────────────────
//  1.  Wi-Fi Credentials
// ─────────────────────────────────────────────
#define WIFI_SSID       "YOUR_WIFI_SSID"
#define WIFI_PASSWORD   "YOUR_WIFI_PASSWORD"
#define WIFI_TIMEOUT_MS 15000   // Max time to wait for connection

// ─────────────────────────────────────────────
//  2.  n8n Webhook Endpoint
// ─────────────────────────────────────────────
//  Replace with your n8n webhook URL.
//  Example: "http://your-n8n-server.com/webhook/edgewake"
#define WEBHOOK_URL     "YOUR_N8N_WEBHOOK_URL"

// ─────────────────────────────────────────────
//  3.  Pin Assignments  (ESP32-CAM / AI-Thinker)
// ─────────────────────────────────────────────

// --- Flame / Smoke Sensor (Tier-1 Tripwire A) ---
// Must be an RTC GPIO for ext1 deep-sleep wake-up.
#define FLAME_SENSOR_PIN   GPIO_NUM_13

// --- Vibration Sensor SW-420 (Tier-1 Tripwire B) ---
// Must be an RTC GPIO.  GPIO 2 = RTC_GPIO 12.
#define VIBRATION_SENSOR_PIN  GPIO_NUM_2

// ext1 bitmask: wake when ANY of these pins go HIGH.
// Bitmask = (1 << GPIO_13) | (1 << GPIO_2)
#define EXT1_WAKEUP_MASK  ((1ULL << FLAME_SENSOR_PIN) | (1ULL << VIBRATION_SENSOR_PIN))

// --- INMP441 I2S Microphone ---
#define I2S_WS_PIN         14   // Word Select (LRCK)
#define I2S_SCK_PIN        15   // Serial Clock (BCLK)
#define I2S_SD_PIN         12   // Serial Data  (DOUT)

// --- On-board Flash LED (ESP32-CAM) ---
#define FLASH_LED_PIN      4

// ─────────────────────────────────────────────
//  4.  Camera Settings
// ─────────────────────────────────────────────
//  AI-Thinker ESP32-CAM uses these fixed pins:
//  PWDN=32, RESET=-1, XCLK=0, SIOD=26, SIOC=27,
//  Y9=35, Y8=34, Y7=39, Y6=36, Y5=21, Y4=19, Y3=18, Y2=5,
//  VSYNC=25, HREF=23, PCLK=22
//
//  Frame size for the capture:
//    FRAMESIZE_QVGA   (320x240)
//    FRAMESIZE_VGA    (640x480)
//    FRAMESIZE_SVGA   (800x600)
//    FRAMESIZE_XGA    (1024x768)
//    FRAMESIZE_SXGA   (1280x1024)
//    FRAMESIZE_UXGA   (1600x1200)   ← highest supported
#define CAM_FRAME_SIZE   FRAMESIZE_SVGA
#define CAM_JPEG_QUALITY 10   // 0–63, lower = better quality

// ─────────────────────────────────────────────
//  5.  Audio / TinyML Thresholds
// ─────────────────────────────────────────────
#define AUDIO_SAMPLE_RATE       16000         // Hz
#define AUDIO_SAMPLE_DURATION   2             // seconds of audio to record
#define AUDIO_BUFFER_SIZE       (AUDIO_SAMPLE_RATE * AUDIO_SAMPLE_DURATION)

// --- Simple Energy-Based Detection ---
// RMS energy above this ⇒ "suspicious sound detected"
// Tune this on-site with Serial output.
#define AUDIO_RMS_THRESHOLD     1500

// --- Edge Impulse TinyML toggle ---
// Set to 1 when you have a trained Edge Impulse model library
// installed.  Set to 0 to fall back to the simple energy detector.
#define USE_TINYML_MODEL  0

// ─────────────────────────────────────────────
//  6.  Alert Metadata
// ─────────────────────────────────────────────
#define DEVICE_ID        "EDGEWAKE-001"
#define DEVICE_LOCATION  "Forest Sector A"

// ─────────────────────────────────────────────
//  7.  Timing
// ─────────────────────────────────────────────
#define SERIAL_BAUD      115200

// Debounce: ignore re-triggers within this window (ms)
#define DEBOUNCE_MS      5000

#endif // CONFIG_H

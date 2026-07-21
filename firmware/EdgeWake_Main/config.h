/*
 * ============================================================
 *  EdgeWake — Simplified Configuration
 * ============================================================
 *  Edit this file before uploading to your ESP32-CAM.
 */

#ifndef CONFIG_H
#define CONFIG_H

// ─────────────────────────────────────────────
//  1.  Wi-Fi Credentials
// ─────────────────────────────────────────────
#define WIFI_SSID       "YOUR_WIFI_SSID"
#define WIFI_PASSWORD   "YOUR_WIFI_PASSWORD"

// ─────────────────────────────────────────────
//  2.  Telegram Bot Settings
// ─────────────────────────────────────────────
//  Create a bot via @BotFather on Telegram to get the Token.
//  Use @userinfobot to get your Chat ID.
#define TELEGRAM_BOT_TOKEN "YOUR_BOT_TOKEN"
#define TELEGRAM_CHAT_ID   "YOUR_CHAT_ID"

// ─────────────────────────────────────────────
//  3.  Pin Assignments
// ─────────────────────────────────────────────

// Sensor 1: Flame Sensor
// We use GPIO 13 because it supports waking the board from Deep Sleep.
#define FLAME_SENSOR_PIN   GPIO_NUM_13

// Sensor 2: Vibration Sensor
// We use GPIO 33 because it's safe (not a boot pin) and also connects 
// to the red LED, which will blink when vibration is detected!
#define VIBRATION_SENSOR_PIN  GPIO_NUM_33

// Microphone Pins (INMP441)
#define I2S_WS_PIN         14   // Word Select (LRCK)
#define I2S_SCK_PIN        15   // Serial Clock (BCLK)
#define I2S_SD_PIN         12   // Serial Data  (DOUT)

// Flashlight (White LED on ESP32-CAM)
#define FLASH_LED_PIN      4

// ─────────────────────────────────────────────
//  4.  Camera Settings
// ─────────────────────────────────────────────
//  FRAMESIZE_SVGA is 800x600 resolution (good balance of quality and speed).
#define CAM_FRAME_SIZE   FRAMESIZE_SVGA
#define CAM_JPEG_QUALITY 10   // 0 (best) to 63 (worst)

#endif

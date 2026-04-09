/*
 * ============================================================
 *  EdgeWake — Camera Utilities
 *  Handles OV2640 init & single-frame JPEG capture
 *  for the AI-Thinker ESP32-CAM board.
 * ============================================================
 */

#ifndef CAMERA_UTILS_H
#define CAMERA_UTILS_H

#include "esp_camera.h"
#include "config.h"

// ── AI-Thinker ESP32-CAM pin map ──────────────────────────
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

/**
 * Initialise the OV2640 camera.
 * Returns true on success.
 */
bool initCamera() {
  camera_config_t config;

  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer   = LEDC_TIMER_0;
  config.pin_d0       = Y2_GPIO_NUM;
  config.pin_d1       = Y3_GPIO_NUM;
  config.pin_d2       = Y4_GPIO_NUM;
  config.pin_d3       = Y5_GPIO_NUM;
  config.pin_d4       = Y6_GPIO_NUM;
  config.pin_d5       = Y7_GPIO_NUM;
  config.pin_d6       = Y8_GPIO_NUM;
  config.pin_d7       = Y9_GPIO_NUM;
  config.pin_xclk     = XCLK_GPIO_NUM;
  config.pin_pclk     = PCLK_GPIO_NUM;
  config.pin_vsync    = VSYNC_GPIO_NUM;
  config.pin_href     = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn     = PWDN_GPIO_NUM;
  config.pin_reset    = RESET_GPIO_NUM;

  config.xclk_freq_hz = 20000000;          // 20 MHz XCLK
  config.pixel_format = PIXFORMAT_JPEG;

  // Use PSRAM if available for larger frame buffers
  if (psramFound()) {
    config.frame_size   = CAM_FRAME_SIZE;
    config.jpeg_quality = CAM_JPEG_QUALITY;
    config.fb_count     = 2;
    config.grab_mode    = CAMERA_GRAB_LATEST;
    config.fb_location  = CAMERA_FB_IN_PSRAM;
    Serial.println("[CAM] PSRAM detected — using high-res capture.");
  } else {
    // No PSRAM: fall back to a smaller frame
    config.frame_size   = FRAMESIZE_VGA;
    config.jpeg_quality = 12;
    config.fb_count     = 1;
    config.fb_location  = CAMERA_FB_IN_DRAM;
    Serial.println("[CAM] No PSRAM — falling back to VGA.");
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("[CAM] Init FAILED (0x%x)\n", err);
    return false;
  }

  // Optional: tweak sensor settings for outdoor clarity
  sensor_t *s = esp_camera_sensor_get();
  if (s) {
    s->set_brightness(s, 1);    // slight brightness boost
    s->set_contrast(s, 1);
    s->set_saturation(s, 0);
    s->set_whitebal(s, 1);      // enable AWB
    s->set_awb_gain(s, 1);
    s->set_exposure_ctrl(s, 1); // auto exposure
    s->set_aec2(s, 1);          // auto exposure DSP
    s->set_gain_ctrl(s, 1);     // auto gain
  }

  Serial.println("[CAM] Initialised OK.");
  return true;
}

/**
 * Capture a single JPEG frame.
 * Turns the flash LED on briefly for illumination.
 *
 * Returns a pointer to the camera frame buffer (caller must
 * call esp_camera_fb_return() when done) or NULL on failure.
 */
camera_fb_t* capturePhoto() {
  Serial.println("[CAM] Capturing photo...");

  // Turn on flash LED
  pinMode(FLASH_LED_PIN, OUTPUT);
  digitalWrite(FLASH_LED_PIN, HIGH);
  delay(150);  // brief warm-up for good exposure

  // Discard one frame to let auto-exposure settle
  camera_fb_t *discard = esp_camera_fb_get();
  if (discard) {
    esp_camera_fb_return(discard);
  }

  // Capture the real frame
  camera_fb_t *fb = esp_camera_fb_get();

  // Turn off flash LED
  digitalWrite(FLASH_LED_PIN, LOW);

  if (!fb) {
    Serial.println("[CAM] Capture FAILED.");
    return NULL;
  }

  Serial.printf("[CAM] Captured %u bytes (%dx%d)\n",
                fb->len, fb->width, fb->height);
  return fb;
}

#endif // CAMERA_UTILS_H

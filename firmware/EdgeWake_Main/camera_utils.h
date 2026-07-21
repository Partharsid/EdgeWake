/*
 * ============================================================
 *  EdgeWake — Camera Utilities
 * ============================================================
 *  This file sets up the OV2640 camera and takes a picture.
 */

#ifndef CAMERA_UTILS_H
#define CAMERA_UTILS_H

#include "esp_camera.h"
#include "config.h"

// These are the internal pins used by the camera on the ESP32-CAM board.
// You do not need to change these!
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

// Function to turn on the camera
bool initCamera() {
  camera_config_t config;
  
  // Connect all the pins to the configuration
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
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG; // We want a standard JPG image

  // If the board has extra memory (PSRAM), use it for high quality
  if (psramFound()) {
    config.frame_size   = CAM_FRAME_SIZE;
    config.jpeg_quality = CAM_JPEG_QUALITY;
    config.fb_count     = 2;
    config.grab_mode    = CAMERA_GRAB_LATEST;
    config.fb_location  = CAMERA_FB_IN_PSRAM;
  } else {
    // If no extra memory, use lower quality
    config.frame_size   = FRAMESIZE_VGA;
    config.jpeg_quality = 12;
    config.fb_count     = 1;
    config.fb_location  = CAMERA_FB_IN_DRAM;
  }

  // Actually start the camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.println("Camera failed to start!");
    return false;
  }

  Serial.println("Camera is ready!");
  return true;
}

// Function to take a picture
camera_fb_t* capturePhoto() {
  Serial.println("Taking a photo...");

  // Turn on the bright white LED so we can see in the dark forest
  pinMode(FLASH_LED_PIN, OUTPUT);
  digitalWrite(FLASH_LED_PIN, HIGH);
  delay(150); // Give the light a moment to brighten up

  // Throw away the first photo (it's often too dark because the camera is adjusting)
  camera_fb_t *discard = esp_camera_fb_get();
  if (discard) {
    esp_camera_fb_return(discard);
  }

  // Take the real photo!
  camera_fb_t *fb = esp_camera_fb_get();

  // Turn off the white LED to save battery
  digitalWrite(FLASH_LED_PIN, LOW);

  if (!fb) {
    Serial.println("Failed to take photo.");
    return NULL;
  }

  Serial.println("Photo taken successfully!");
  return fb;
}

#endif

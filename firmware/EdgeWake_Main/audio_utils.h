/*
 * ============================================================
 *  EdgeWake — Audio & AI Utilities
 * ============================================================
 *  This file sets up the microphone and runs the Edge Impulse 
 *  Artificial Intelligence to figure out if it hears fire or a chainsaw.
 */

#ifndef AUDIO_UTILS_H
#define AUDIO_UTILS_H

#include <driver/i2s.h>
#include "config.h"
#include "src/EdgeWake-Forest-Audio_inferencing/src/EdgeWake-Forest-Audio_inferencing.h"

// The ESP32 has multiple audio ports. We use port 1 because the camera uses port 0.
#define I2S_PORT I2S_NUM_1

// -------------------------------------------------------------
// 1. Turn on the Microphone
// -------------------------------------------------------------
bool initMicrophone() {
  Serial.println("Turning on Microphone...");

  // Setup the digital audio format (I2S)
  const i2s_config_t i2s_config = {
    .mode                 = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate          = 16000, // 16kHz is perfect for human and machine hearing
    .bits_per_sample      = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format       = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags     = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count        = 8,
    .dma_buf_len          = 1024,
    .use_apll             = false,
    .tx_desc_auto_clear   = false,
    .fixed_mclk           = 0
  };

  // Connect the microphone to the pins defined in config.h
  const i2s_pin_config_t pin_config = {
    .bck_io_num   = I2S_SCK_PIN,
    .ws_io_num    = I2S_WS_PIN,
    .data_out_num = I2S_PIN_NO_CHANGE, 
    .data_in_num  = I2S_SD_PIN
  };

  if (i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL) != ESP_OK) return false;
  if (i2s_set_pin(I2S_PORT, &pin_config) != ESP_OK) return false;
  
  // Clear any garbage noise from startup
  i2s_zero_dma_buffer(I2S_PORT);
  delay(100);

  Serial.println("Microphone is ready!");
  return true;
}

// -------------------------------------------------------------
// 2. Turn off the Microphone
// -------------------------------------------------------------
void deinitMicrophone() {
  i2s_driver_uninstall(I2S_PORT);
}

// -------------------------------------------------------------
// 3. Record Audio and check it with Artificial Intelligence
// -------------------------------------------------------------
// Returns 'true' if the AI hears a fire or chainsaw.
// Returns 'false' if it's just normal forest background noise.
bool verifyThreatAudio() {
  // We need to record 2 seconds of audio at 16,000 samples per second.
  // 16,000 * 2 = 32,000 samples.
  int total_samples = 32000;
  
  // Allocate memory for the recording. We use ps_malloc to use the extra PSRAM memory.
  int16_t *audioBuffer = (int16_t *)ps_malloc(total_samples * sizeof(int16_t));
  
  if (!audioBuffer) {
    Serial.println("Out of memory for audio! Assuming there is a threat just to be safe.");
    return true; 
  }

  Serial.println("Recording 2 seconds of audio...");
  size_t bytesRead = 0;
  
  // Read the audio from the microphone in a loop until we have 2 seconds
  for (int i = 0; i < total_samples; i += 512) {
    i2s_read(I2S_PORT, &audioBuffer[i], 512 * sizeof(int16_t), &bytesRead, portMAX_DELAY);
  }

  Serial.println("Recording finished. Asking AI to analyze it...");

  // Send the audio buffer to the Edge Impulse AI Model
  signal_t signal;
  numpy::signal_from_buffer(audioBuffer, total_samples, &signal);

  ei_impulse_result_t result = { 0 };
  run_classifier(&signal, &result, false);

  // We are looking for "fire" or "chainsaw"
  float fireScore = 0.0;
  float chainsawScore = 0.0;

  // Loop through all the things the AI knows how to identify
  for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
    String label = result.classification[ix].label;
    float score = result.classification[ix].value;
    
    Serial.print("AI thinks it is '");
    Serial.print(label);
    Serial.print("': ");
    Serial.println(score); // 1.0 means 100% sure, 0.0 means 0% sure

    if (label == "fire") fireScore = score;
    if (label == "chainsaw") chainsawScore = score;
  }

  // Free the memory so the camera can use it later
  free(audioBuffer);

  // If the AI is more than 60% sure it heard a fire or chainsaw, it's a threat!
  if (fireScore > 0.6 || chainsawScore > 0.6) {
    Serial.println("⚠️ DANGER: Fire or Chainsaw confirmed by AI!");
    return true; 
  } else {
    Serial.println("✅ False Alarm. Just normal background noise.");
    return false;
  }
}

#endif

/*
 * ============================================================
 *  EdgeWake — Audio Utilities
 *  INMP441 I2S driver  +  threat-verification logic
 *
 *  Two modes (selected via USE_TINYML_MODEL in config.h):
 *   0  →  Simple RMS energy detector  (demo-ready)
 *   1  →  Edge Impulse TinyML inference (requires trained
 *          model library — see docs/SETUP_GUIDE.md)
 * ============================================================
 */

#ifndef AUDIO_UTILS_H
#define AUDIO_UTILS_H

#include <driver/i2s.h>
#include "config.h"

// If you have a trained Edge Impulse model, uncomment and
// include its header here:
// #include <Your_Edge_Impulse_inferencing.h>

// I2S port to use (ESP32 has two: I2S_NUM_0, I2S_NUM_1)
// The camera occupies I2S_NUM_0 internally, so we use I2S_NUM_1.
#define I2S_PORT  I2S_NUM_1

// Intermediate read-chunk size (in bytes)
#define I2S_READ_CHUNK  1024

/**
 * Initialise the I2S peripheral for the INMP441 microphone.
 * Returns true on success.
 */
bool initMicrophone() {
  Serial.println("[MIC] Initialising I2S...");

  const i2s_config_t i2s_config = {
    .mode                 = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate          = AUDIO_SAMPLE_RATE,
    .bits_per_sample      = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format       = I2S_CHANNEL_FMT_ONLY_LEFT,   // L/R pin tied to GND
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags     = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count        = 8,
    .dma_buf_len          = 1024,
    .use_apll             = false,
    .tx_desc_auto_clear   = false,
    .fixed_mclk           = 0
  };

  const i2s_pin_config_t pin_config = {
    .bck_io_num   = I2S_SCK_PIN,
    .ws_io_num    = I2S_WS_PIN,
    .data_out_num = I2S_PIN_NO_CHANGE,  // not transmitting
    .data_in_num  = I2S_SD_PIN
  };

  esp_err_t err;

  err = i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
  if (err != ESP_OK) {
    Serial.printf("[MIC] Driver install FAILED (0x%x)\n", err);
    return false;
  }

  err = i2s_set_pin(I2S_PORT, &pin_config);
  if (err != ESP_OK) {
    Serial.printf("[MIC] Pin config FAILED (0x%x)\n", err);
    return false;
  }

  // Flush initial garbage from DMA buffers
  i2s_zero_dma_buffer(I2S_PORT);
  delay(100);

  Serial.println("[MIC] Initialised OK.");
  return true;
}

/**
 * Shut down the I2S peripheral to save power before sleep.
 */
void deinitMicrophone() {
  i2s_driver_uninstall(I2S_PORT);
  Serial.println("[MIC] Driver uninstalled.");
}

/**
 * Record `AUDIO_SAMPLE_DURATION` seconds of 16-bit mono audio
 * into the provided buffer (must be at least AUDIO_BUFFER_SIZE
 * int16_t elements).
 *
 * Returns the number of samples actually read.
 */
int recordAudio(int16_t *buffer, int maxSamples) {
  Serial.println("[MIC] Recording audio...");
  int totalSamples = 0;
  size_t bytesRead = 0;

  // Temporary chunk buffer
  int16_t chunk[I2S_READ_CHUNK / 2];

  unsigned long startMs = millis();
  unsigned long durationMs = AUDIO_SAMPLE_DURATION * 1000UL;

  while ((millis() - startMs) < durationMs && totalSamples < maxSamples) {
    esp_err_t result = i2s_read(I2S_PORT, chunk, I2S_READ_CHUNK,
                                &bytesRead, portMAX_DELAY);
    if (result == ESP_OK && bytesRead > 0) {
      int samplesInChunk = bytesRead / 2;  // 16-bit = 2 bytes
      for (int i = 0; i < samplesInChunk && totalSamples < maxSamples; i++) {
        buffer[totalSamples++] = chunk[i];
      }
    }
  }

  Serial.printf("[MIC] Recorded %d samples (%.1f sec)\n",
                totalSamples,
                (float)totalSamples / AUDIO_SAMPLE_RATE);
  return totalSamples;
}

// ────────────────────────────────────────────────────────────
//  Option A:  Simple RMS Energy Detector (default)
// ────────────────────────────────────────────────────────────

/**
 * Compute the Root Mean Square energy of the audio buffer.
 */
float computeRMS(const int16_t *buffer, int numSamples) {
  double sumSq = 0;
  for (int i = 0; i < numSamples; i++) {
    double s = (double)buffer[i];
    sumSq += s * s;
  }
  return (float)sqrt(sumSq / numSamples);
}

/**
 * Analyse an audio buffer with the simple energy approach.
 * Returns true if RMS exceeds the configured threshold.
 */
bool analyseAudioSimple(const int16_t *buffer, int numSamples) {
  float rms = computeRMS(buffer, numSamples);
  Serial.printf("[AUDIO] RMS energy = %.1f  (threshold = %d)\n",
                rms, AUDIO_RMS_THRESHOLD);
  return (rms >= AUDIO_RMS_THRESHOLD);
}

// ────────────────────────────────────────────────────────────
//  Option B:  Edge Impulse TinyML Inference (opt-in)
// ────────────────────────────────────────────────────────────
//
//  To use this:
//  1.  Train a model on Edge Impulse with classes like
//      "fire", "chainsaw", "background".
//  2.  Export as an Arduino library and install it.
//  3.  Uncomment the #include above and set
//      USE_TINYML_MODEL  1  in config.h.
//

#if USE_TINYML_MODEL
bool analyseAudioTinyML(const int16_t *buffer, int numSamples) {
  Serial.println("[ML] Running TinyML inference...");

  /*
   * ── Edge Impulse integration point ──
   *
   * // 1. Create a signal from the audio buffer
   * signal_t signal;
   * int err = numpy::signal_from_buffer(buffer, numSamples, &signal);
   * if (err != 0) {
   *   Serial.println("[ML] Signal creation failed");
   *   return false;
   * }
   *
   * // 2. Run the classifier
   * ei_impulse_result_t result = { 0 };
   * err = run_classifier(&signal, &result, false);
   * if (err != EI_IMPULSE_OK) {
   *   Serial.printf("[ML] Classifier failed (%d)\n", err);
   *   return false;
   * }
   *
   * // 3. Evaluate the results
   * float fireScore      = 0.0;
   * float chainsawScore  = 0.0;
   *
   * for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
   *   Serial.printf("[ML]  %s: %.5f\n",
   *                 result.classification[ix].label,
   *                 result.classification[ix].value);
   *   if (strcmp(result.classification[ix].label, "fire") == 0) {
   *     fireScore = result.classification[ix].value;
   *   }
   *   if (strcmp(result.classification[ix].label, "chainsaw") == 0) {
   *     chainsawScore = result.classification[ix].value;
   *   }
   * }
   *
   * // Threat if fire OR chainsaw confidence > 0.6
   * return (fireScore > 0.6 || chainsawScore > 0.6);
   */

  // Placeholder — replace with the code above once your model
  // library is installed.
  Serial.println("[ML] TinyML model not yet linked. Falling back to energy.");
  return analyseAudioSimple(buffer, numSamples);
}
#endif

/**
 * Top-level audio verification function.
 * Automatically picks the right analysis path based on config.
 *
 * Returns true  → threat verified (proceed to camera capture)
 * Returns false → false alarm (go back to sleep)
 */
bool verifyThreatAudio() {
  // Allocate audio buffer (in PSRAM if available)
  int16_t *audioBuffer;
  if (psramFound()) {
    audioBuffer = (int16_t *)ps_malloc(AUDIO_BUFFER_SIZE * sizeof(int16_t));
  } else {
    audioBuffer = (int16_t *)malloc(AUDIO_BUFFER_SIZE * sizeof(int16_t));
  }

  if (!audioBuffer) {
    Serial.println("[AUDIO] Buffer allocation FAILED — assuming threat.");
    return true;  // fail-open: if we can't verify, assume worst case
  }

  // Record audio
  int numSamples = recordAudio(audioBuffer, AUDIO_BUFFER_SIZE);

  // Analyse
  bool threatDetected = false;

  #if USE_TINYML_MODEL
    threatDetected = analyseAudioTinyML(audioBuffer, numSamples);
  #else
    threatDetected = analyseAudioSimple(audioBuffer, numSamples);
  #endif

  free(audioBuffer);

  if (threatDetected) {
    Serial.println("[AUDIO] ✓ Threat VERIFIED by audio analysis.");
  } else {
    Serial.println("[AUDIO] ✗ False alarm — no threat signature found.");
  }

  return threatDetected;
}

#endif // AUDIO_UTILS_H

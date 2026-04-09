/*
 * ============================================================
 *  EdgeWake — Network Utilities
 *  Wi-Fi connection + multipart HTTP POST to n8n webhook
 * ============================================================
 */

#ifndef NETWORK_UTILS_H
#define NETWORK_UTILS_H

#include <WiFi.h>
#include <HTTPClient.h>
#include "config.h"
#include "esp_camera.h"

/**
 * Connect to the configured Wi-Fi network.
 * Returns true if connected within WIFI_TIMEOUT_MS.
 */
bool connectWiFi() {
  Serial.printf("[NET] Connecting to '%s'", WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - start > WIFI_TIMEOUT_MS) {
      Serial.println("\n[NET] Wi-Fi TIMEOUT — could not connect.");
      return false;
    }
    delay(250);
    Serial.print(".");
  }

  Serial.printf("\n[NET] Connected!  IP: %s  RSSI: %d dBm\n",
                WiFi.localIP().toString().c_str(),
                WiFi.RSSI());
  return true;
}

/**
 * Disconnect Wi-Fi and turn off the radio to save power.
 */
void disconnectWiFi() {
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  Serial.println("[NET] Wi-Fi OFF.");
}

/**
 * Send the captured photo + metadata to the n8n webhook.
 *
 * The request is a standard multipart/form-data POST:
 *   Part 1  "image"      →  the JPEG binary
 *   Part 2  "alert_type" →  e.g. "Fire"
 *   Part 3  "device_id"  →  e.g. "EDGEWAKE-001"
 *   Part 4  "location"   →  e.g. "Forest Sector A"
 *   Part 5  "rms_energy" →  audio RMS value (for logging)
 *   Part 6  "timestamp"  →  millis since boot
 *
 * Returns true if the server responded with HTTP 2xx.
 */
bool sendAlert(camera_fb_t *fb, const char *alertType, float rmsEnergy) {
  if (!fb) {
    Serial.println("[NET] No photo to send.");
    return false;
  }

  Serial.println("[NET] Sending alert to n8n...");

  HTTPClient http;
  http.begin(WEBHOOK_URL);
  http.setTimeout(20000);  // 20-second timeout

  // ── Build multipart body ──────────────────────────────
  String boundary = "----EdgeWakeBoundary" + String(millis());
  String contentType = "multipart/form-data; boundary=" + boundary;
  http.addHeader("Content-Type", contentType);

  // Helper lambdas for cleaner part construction
  String head = "";
  String tail = "\r\n--" + boundary + "--\r\n";

  // Part: alert_type
  head += "--" + boundary + "\r\n";
  head += "Content-Disposition: form-data; name=\"alert_type\"\r\n\r\n";
  head += String(alertType) + "\r\n";

  // Part: device_id
  head += "--" + boundary + "\r\n";
  head += "Content-Disposition: form-data; name=\"device_id\"\r\n\r\n";
  head += String(DEVICE_ID) + "\r\n";

  // Part: location
  head += "--" + boundary + "\r\n";
  head += "Content-Disposition: form-data; name=\"location\"\r\n\r\n";
  head += String(DEVICE_LOCATION) + "\r\n";

  // Part: rms_energy
  head += "--" + boundary + "\r\n";
  head += "Content-Disposition: form-data; name=\"rms_energy\"\r\n\r\n";
  head += String(rmsEnergy, 1) + "\r\n";

  // Part: timestamp
  head += "--" + boundary + "\r\n";
  head += "Content-Disposition: form-data; name=\"timestamp\"\r\n\r\n";
  head += String(millis()) + "\r\n";

  // Part: image (binary JPEG)
  head += "--" + boundary + "\r\n";
  head += "Content-Disposition: form-data; name=\"image\"; filename=\"alert.jpg\"\r\n";
  head += "Content-Type: image/jpeg\r\n\r\n";

  // Calculate total body length
  uint32_t totalLen = head.length() + fb->len + tail.length();

  // ── Stream the multipart body ─────────────────────────
  // We use a raw WiFiClient to stream large images without
  // loading everything into RAM at once.
  WiFiClient *stream = http.getStreamPtr();

  // Start the POST manually
  http.addHeader("Content-Length", String(totalLen));

  // Use sendRequest with assembled body parts
  uint8_t *body = (uint8_t *)malloc(totalLen);
  if (!body) {
    Serial.println("[NET] Body allocation FAILED — sending without streaming.");
    // Fall back to a simpler (non-image) alert
    http.addHeader("Content-Type", "application/json");
    String json = "{\"alert_type\":\"" + String(alertType) +
                  "\",\"device_id\":\"" + String(DEVICE_ID) +
                  "\",\"location\":\"" + String(DEVICE_LOCATION) +
                  "\",\"rms_energy\":" + String(rmsEnergy, 1) +
                  ",\"error\":\"image_too_large_for_ram\"}";
    int code = http.POST(json);
    Serial.printf("[NET] Fallback JSON response: %d\n", code);
    http.end();
    return (code >= 200 && code < 300);
  }

  // Assemble the full body in the allocated buffer
  uint32_t offset = 0;
  memcpy(body + offset, head.c_str(), head.length());
  offset += head.length();
  memcpy(body + offset, fb->buf, fb->len);
  offset += fb->len;
  memcpy(body + offset, tail.c_str(), tail.length());

  int httpCode = http.POST(body, totalLen);
  free(body);

  Serial.printf("[NET] Server responded: %d\n", httpCode);

  if (httpCode >= 200 && httpCode < 300) {
    String response = http.getString();
    Serial.printf("[NET] Response body: %s\n", response.c_str());
    http.end();
    return true;
  } else {
    Serial.printf("[NET] Upload FAILED (HTTP %d)\n", httpCode);
    http.end();
    return false;
  }
}

#endif // NETWORK_UTILS_H

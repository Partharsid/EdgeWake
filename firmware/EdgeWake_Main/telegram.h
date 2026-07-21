/*
 * ============================================================
 *  EdgeWake — Telegram Integration
 * ============================================================
 *  This file handles connecting to Wi-Fi and sending the 
 *  photo alert directly to your phone via Telegram.
 */

#ifndef TELEGRAM_H
#define TELEGRAM_H

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "esp_camera.h"
#include "config.h"

// This function connects to WiFi, sends the picture to Telegram, and disconnects.
bool sendAlertToTelegram(camera_fb_t *fb, String message) {
  Serial.print("Connecting to WiFi: ");
  Serial.println(WIFI_SSID);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  // Wait up to 10 seconds for WiFi
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  Serial.println();
  
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Failed to connect to WiFi.");
    WiFi.mode(WIFI_OFF);
    return false;
  }
  
  Serial.println("WiFi connected! Sending to Telegram...");

  // Set up a secure connection to Telegram (HTTPS)
  WiFiClientSecure client;
  client.setInsecure(); // We don't check the SSL certificate to keep code simple
  
  if (!client.connect("api.telegram.org", 443)) {
    Serial.println("Failed to connect to Telegram server.");
    WiFi.mode(WIFI_OFF);
    return false;
  }

  // We have to build a "multipart/form-data" HTTP request.
  // This is how websites upload files.
  String boundary = "----EdgeWakeBoundary123456";
  
  // 1. First part of the body (The text message)
  String head = "--" + boundary + "\r\n";
  head += "Content-Disposition: form-data; name=\"chat_id\"\r\n\r\n";
  head += String(TELEGRAM_CHAT_ID) + "\r\n";
  
  head += "--" + boundary + "\r\n";
  head += "Content-Disposition: form-data; name=\"caption\"\r\n\r\n";
  head += message + "\r\n";
  
  // 2. Second part of the body (The start of the image)
  head += "--" + boundary + "\r\n";
  head += "Content-Disposition: form-data; name=\"photo\"; filename=\"alert.jpg\"\r\n";
  head += "Content-Type: image/jpeg\r\n\r\n";

  // 3. The end of the body
  String tail = "\r\n--" + boundary + "--\r\n";

  // Calculate the total size of the message
  uint32_t totalLength = head.length() + fb->len + tail.length();

  // Send the HTTP Headers to Telegram
  client.println("POST /bot" + String(TELEGRAM_BOT_TOKEN) + "/sendPhoto HTTP/1.1");
  client.println("Host: api.telegram.org");
  client.println("Content-Type: multipart/form-data; boundary=" + boundary);
  client.print("Content-Length: ");
  client.println(totalLength);
  client.println();

  // Send the body (Streamed to avoid running out of memory!)
  client.print(head);                  // Send text & headers
  client.write(fb->buf, fb->len);      // Send the actual image bytes
  client.print(tail);                  // Send the closing boundary

  // Wait for Telegram to respond
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      break; // End of headers
    }
  }
  
  // Read the actual response from Telegram
  String response = client.readStringUntil('\n');
  Serial.println("Telegram Response: " + response);
  
  client.stop();
  WiFi.mode(WIFI_OFF); // Turn off WiFi to save battery
  
  return true;
}

#endif

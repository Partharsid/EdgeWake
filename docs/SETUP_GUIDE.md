# 🚀 EdgeWake — Simplified Setup Guide

Step-by-step instructions to get the simplified EdgeWake running from zero.

---

## Prerequisites

| Tool | Purpose | Download |
|:---|:---|:---|
| **Arduino IDE 2.x** | Compile & upload firmware | [arduino.cc/en/software](https://www.arduino.cc/en/software) |
| **FTDI USB-to-TTL** | Program the ESP32-CAM | Any CP2102 or CH340 module |
| **Telegram** | Alert notifications | App on your phone |

---

## Part 1: Telegram Setup (The easy way!)

Instead of complex cloud servers, EdgeWake now talks directly to your phone via Telegram.

1. Open Telegram on your phone or computer.
2. Search for the user **@BotFather** (this is the official bot maker).
3. Send the message `/newbot` and follow the instructions to give your bot a name.
4. BotFather will give you an **API Token** (it looks like `123456:ABC-DEF1234ghIkl-zyx57W2v1u123ew11`). **Save this.**
5. Now, search for **@userinfobot** in Telegram and send `/start`.
6. It will reply with your **Id** (a number like `123456789`). **Save this Chat ID.**
7. Open a chat with your new bot (search for the name you gave it) and click **Start** so it has permission to message you.

---

## Part 2: Arduino IDE Setup

### 1. Install ESP32 Board Package

1. Open **Arduino IDE → File → Preferences**
2. In "Additional Board URLs", add: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
3. Go to **Tools → Board → Boards Manager**, search for **esp32**, and install.

### 2. Select Board Settings

| Setting | Value |
|:---|:---|
| Board | **AI Thinker ESP32-CAM** |
| Upload Speed | **115200** |
| PSRAM | **Enabled** ⚠️ Critical! |
| Port | (your FTDI COM port) |

### 3. Open & Configure the Firmware

1. Open `firmware/EdgeWake_Main/EdgeWake_Main.ino`
2. Open the `config.h` tab.
3. Edit these 4 lines:

```cpp
#define WIFI_SSID       "your_wifi_name"
#define WIFI_PASSWORD   "your_wifi_password"
#define TELEGRAM_BOT_TOKEN "123456:ABC-DEF1234ghIkl-zyx57W2v1u123ew11"
#define TELEGRAM_CHAT_ID   "123456789"
```

### 4. Upload

1. Wire the FTDI programmer (see `WIRING_GUIDE.md`).
2. Connect **IO0 → GND** on the ESP32-CAM.
3. Press the **RST** button on the back.
4. Click **Upload** in Arduino IDE.
5. When it says "Done uploading", **remove** the IO0-GND wire.
6. Press **RST** again to run!

---

## Part 3: Live Demo

1. Open the **Serial Monitor** in Arduino IDE (set to 115200 baud).
2. You should see it say `Entering Deep Sleep... Zzz... 🌙`.
3. Hold a **lighter** near the flame sensor or tap the vibration sensor.
4. Watch the Serial Monitor as it walks through the 5 Tiers.
5. Check your phone! You should receive a photo from your Telegram Bot!

---

## FAQ

**Q: The camera image is dark / purple.**
A: Make sure you Enabled **PSRAM** in the Arduino IDE Tools menu before uploading.

**Q: The board doesn't turn on!**
A: Unplug the wire going to GPIO 12, turn the board on, and plug it back in. GPIO 12 is a special boot pin.

**Q: The sensor triggers too easily.**
A: Turn the small blue screw on the sensor to adjust the sensitivity.

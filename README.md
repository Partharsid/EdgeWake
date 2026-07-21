# 🌲 EdgeWake — Tiered Cascade Guardian (Beginner-Friendly Edition)

> Ultra-low-power Edge AI system for real-time forest threat detection.  
> *"The smartest sensor is the one that knows when NOT to work."*

![ESP32](https://img.shields.io/badge/ESP32--CAM-AI--Thinker-green?style=flat-square)
![Telegram](https://img.shields.io/badge/Alerts-Telegram-blue?style=flat-square)
![License](https://img.shields.io/badge/License-MIT-blue?style=flat-square)

---

## What is EdgeWake?

EdgeWake is a battery-powered forest surveillance node that **sleeps 99% of the time** and only wakes up when a real threat is detected. It uses a 5-tier cascade architecture to progressively verify threats.

**This is the heavily simplified, beginner-friendly version of EdgeWake.** The code has been rewritten with extensive comments, hardware bugs have been fixed, and complex cloud integrations have been replaced with a simple Telegram bot.

### Key Features

- ⚡ **5-Tier Cascade** — Deep Sleep → Hardware Interrupt → Audio Verify → Camera Capture → Telegram
- 🔥 **Dual Sensor Wake-Up** — Flame sensor (GPIO 13) + Vibration sensor (GPIO 33)
- 🧠 **TinyML AI** — Audio verification using a 1D Convolutional Neural Network via Edge Impulse
- 📷 **Evidence Capture** — OV2640 camera takes a JPG photo before alerting
- ☁️ **Telegram Alerts** — Sends the photo and threat data directly to your phone via Telegram!
- 🔋 **Ultra-Low Power** — Designed to sleep to save battery.

---

## Architecture

```
┌─────────────┐     ┌──────────────┐     ┌───────────────┐     ┌────────────────┐     ┌──────────────┐
│   TIER 1    │     │    TIER 2    │     │    TIER 3     │     │    TIER 4      │     │    TIER 5    │
│ Deep Sleep  │────▶│  Hardware    │────▶│    Audio      │────▶│   Camera       │────▶│  Telegram    │
│   ~10 µA    │     │  Interrupt   │     │ Verification  │     │   Capture      │     │   Alert      │
│             │     │              │     │   (AI checks) │     │                │     │              │
└─────────────┘     └──────────────┘     └───────────────┘     └────────────────┘     └──────────────┘
     ▲                                                                                       │
     └───────────────────────────── Back to Sleep ◀──────────────────────────────────────────┘
```

---

## Hardware Needed

| Component | Model | Purpose |
|:---|:---|:---|
| Microcontroller | ESP32-CAM (AI-Thinker) | Main brain + camera |
| Programmer | FTDI CP2102 USB-to-TTL | Upload code to ESP32 |
| Microphone | INMP441 | AI Audio verification |
| Flame Sensor | KY-026 | Fire detection tripwire |
| Vibration Sensor | SW-420 | Logging/chainsaw tripwire |
| Power | 18650 Battery + TP4056 | Battery supply |

---

## Quick Start

### 1. Setup Telegram
1. Open the Telegram app and search for `@BotFather`.
2. Type `/newbot`, follow the instructions, and save your **Bot Token**.
3. Search for `@userinfobot`, type `/start`, and save your **Chat ID**.

### 2. Configure the Code
1. Open `firmware/EdgeWake_Main/EdgeWake_Main.ino` in the Arduino IDE.
2. Open the `config.h` tab.
3. Put in your WiFi Name, WiFi Password, Telegram Bot Token, and Telegram Chat ID.

### 3. Flash the Firmware
1. Select Board: "AI Thinker ESP32-CAM".
2. Turn on PSRAM in the Tools menu.
3. Connect your FTDI programmer. Connect IO0 to GND.
4. Click Upload!

### 4. Wire the Hardware
See [`docs/WIRING_GUIDE.md`](docs/WIRING_GUIDE.md) for the complete simple wiring guide.

---

## License

MIT — Use it, modify it, deploy it. Just keep the forests safe. 🌲

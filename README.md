# 🌲 EdgeWake — Tiered Cascade Guardian

> Ultra-low-power Edge AI system for real-time forest threat detection.  
> *"The smartest sensor is the one that knows when NOT to work."*

![ESP32](https://img.shields.io/badge/ESP32--CAM-AI--Thinker-green?style=flat-square)
![n8n](https://img.shields.io/badge/Automation-n8n-orange?style=flat-square)
![License](https://img.shields.io/badge/License-MIT-blue?style=flat-square)

---

## What is EdgeWake?

EdgeWake is a battery-powered forest surveillance node that **sleeps 99% of the time** and only wakes up when a real threat is detected. It uses a 5-tier cascade architecture to progressively verify threats — from a near-zero-power hardware interrupt, through audio AI verification, to camera capture and cloud notification — all in under 5 seconds.

### Key Features

- ⚡ **5-Tier Cascade** — Deep Sleep → Hardware Interrupt → Audio Verify → Camera Capture → Cloud Handoff
- 🔥 **Dual Sensor Wake-Up** — Flame sensor (GPIO 13) + Vibration sensor (GPIO 2) via ext1 multi-pin interrupt
- 🧠 **95% Accuracy TinyML** — Audio verification using a 1D Convolutional Neural Network (MFCC features)
- 🎙️ **Edge AI Verification** — Ultra-fast 7ms on-device inference for fire and chainsaw detection
- 📷 **Evidence Capture** — OV2640 camera takes JPEG proof before alerting
- ☁️ **Automated Alerts** — n8n workflow routes alerts to Google Drive + Telegram with photos
- 🔋 **Ultra-Low Power** — ~115 µA optimized sleep current → 2+ years on a single 18650 battery
- 📊 **Live Dashboard** — Glassmorphism-themed monitoring UI with real-time cascade simulation

---

## Architecture

```
┌─────────────┐     ┌──────────────┐     ┌───────────────┐     ┌────────────────┐     ┌──────────────┐
│   TIER 1    │     │    TIER 2    │     │    TIER 3     │     │    TIER 4      │     │    TIER 5    │
│ Deep Sleep  │────▶│  Hardware    │────▶│    Audio      │────▶│   Camera       │────▶│    Cloud     │
│   ~10 µA    │     │  Interrupt   │     │ Verification  │     │   Capture      │     │   Handoff    │
│             │     │  ~20 mA      │     │   ~40 mA      │     │   ~120 mA      │     │   ~160 mA    │
└─────────────┘     └──────────────┘     └───────────────┘     └────────────────┘     └──────────────┘
     ▲                                                                                       │
     └───────────────────────────── Back to Sleep ◀──────────────────────────────────────────┘
```

---

## Hardware BOM

| Component | Model | Purpose | Cost |
|:---|:---|:---|:---:|
| Microcontroller | ESP32-CAM (AI-Thinker) | Main brain + OV2640 camera | ~₹500 |
| Programmer | FTDI CP2102 USB-to-TTL | Upload firmware | ~₹150 |
| Microphone | INMP441 I2S MEMS | Audio verification | ~₹200 |
| Flame Sensor | KY-026 IR Module | Fire detection tripwire | ~₹50 |
| Vibration Sensor | SW-420 | Vibration/logging tripwire | ~₹40 |
| Power | 18650 + TP4056 | Battery supply | ~₹200 |
| **Total** | | | **~₹1,140** |

---

## Project Structure

```
EdgeWake/
├── firmware/
│   └── EdgeWake_Main/
│       ├── EdgeWake_Main.ino    # Main cascade logic
│       ├── config.h             # All tunable parameters
│       ├── camera_utils.h       # OV2640 camera driver
│       ├── audio_utils.h        # INMP441 I2S + threat analysis
│       ├── network_utils.h      # WiFi + HTTP POST upload
│       └── src/                 # Integrated TinyML Model (Edge Impulse)
├── n8n_workflow/
│   └── edgewake_workflow.json   # Importable n8n automation
├── dashboard/
│   ├── index.html               # Monitoring UI
│   ├── style.css                # Dark glassmorphism theme
│   └── script.js                # Simulation engine
├── docs/
│   ├── WIRING_GUIDE.md          # Pin-by-pin wiring diagram
│   ├── SETUP_GUIDE.md           # Full deployment instructions
│   ├── BATTERY_ANALYSIS.md      # Research-backed power analysis
│   └── TINYML_TRAINING_GUIDE.md # Guide for retraining the model
└── scripts/
    └── download_training_data.ps1 # Dataset generator
```

---

## Quick Start

### 1. Flash the Firmware
```bash
# Open firmware/EdgeWake_Main/EdgeWake_Main.ino in Arduino IDE
# Board: "AI Thinker ESP32-CAM" | PSRAM: Enabled
# Edit config.h with your WiFi credentials + n8n webhook URL
# Upload via FTDI (IO0 → GND during flash)
```

### 2. Import n8n Workflow
```
Drag n8n_workflow/edgewake_workflow.json into your n8n canvas
Set up Telegram Bot + Google Drive credentials
Activate the workflow
```

### 3. Wire the Hardware
See [`docs/WIRING_GUIDE.md`](docs/WIRING_GUIDE.md) for the complete pin-by-pin guide.

### 4. Demo the Dashboard
```bash
# Open dashboard/index.html in any browser
# Click "Simulate Fire" or "Simulate Vibration"
# Watch the 5-tier cascade execute in real-time
```

---

## Battery Life (Researched)

| Build Level | Sleep Current | Battery Life (3000 mAh) |
|:---|:---:|:---:|
| Stock (modules as-is) | ~32.5 mA | ~3 days |
| Bare sensors (no module boards) | ~2.55 mA | ~5 weeks |
| Full optimization (LDO swap) | ~115 µA | ~2 years |
| Solar + optimized | ~115 µA + solar | Indefinite |

Full analysis with formulas and sources: [`docs/BATTERY_ANALYSIS.md`](docs/BATTERY_ANALYSIS.md)

---

## Tech Stack

- **Edge:** Arduino C++ (ESP-IDF: esp_camera, driver/i2s, esp_sleep)
- **AI:** Edge Impulse TinyML (1D CNN Model, 95.1% accuracy, 7ms latency)
- **Cloud:** n8n (Webhook → Switch → Google Drive + Telegram)
- **Dashboard:** Vanilla HTML5 + CSS3 + JavaScript

---

## License

MIT — Use it, modify it, deploy it. Just keep the forests safe. 🌲

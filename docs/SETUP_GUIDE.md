# 🚀 EdgeWake — Setup & Deployment Guide

Step-by-step instructions to get EdgeWake running from zero.

---

## Prerequisites

| Tool | Purpose | Download |
|:---|:---|:---|
| **Arduino IDE 2.x** | Compile & upload firmware | [arduino.cc/en/software](https://www.arduino.cc/en/software) |
| **FTDI USB-to-TTL** | Program the ESP32-CAM | Any CP2102 or CH340 module |
| **n8n** | Cloud automation engine | [n8n.io](https://n8n.io) (cloud or self-hosted) |
| **Telegram** | Alert notifications | Create a bot via @BotFather |

---

## Part 1: Arduino IDE Setup

### 1.1 Install ESP32 Board Package

1. Open **Arduino IDE → File → Preferences**
2. In "Additional Board URLs", add:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
3. Go to **Tools → Board → Boards Manager**
4. Search for **esp32** by Espressif Systems
5. Click **Install** (version 2.x or 3.x)

### 1.2 Select Board Settings

| Setting | Value |
|:---|:---|
| Board | **AI Thinker ESP32-CAM** |
| Upload Speed | **115200** |
| CPU Frequency | **240 MHz** |
| Flash Frequency | **80 MHz** |
| Flash Mode | **QIO** |
| Partition Scheme | **Huge APP (3MB No OTA / 1MB SPIFFS)** |
| PSRAM | **Enabled** ⚠️ Critical! |
| Port | (your FTDI COM port) |

### 1.3 Open & Configure the Firmware

1. Open the file: `firmware/EdgeWake_Main/EdgeWake_Main.ino`
2. Open `config.h` (it will appear as a tab in Arduino IDE)
3. Edit these values:

```cpp
#define WIFI_SSID       "your_wifi_name"
#define WIFI_PASSWORD   "your_wifi_password"
#define WEBHOOK_URL     "https://your-n8n.com/webhook/edgewake"
```

### 1.4 Upload

1. Wire the FTDI programmer (see `WIRING_GUIDE.md`)
2. Connect **IO0 → GND** on the ESP32-CAM
3. Press **RST** button
4. Click **Upload** in Arduino IDE
5. Wait for "Done uploading"
6. **Remove** the IO0-GND jumper
7. Press **RST** to run

### 1.5 Verify

1. Open **Serial Monitor** (115200 baud)
2. You should see:
   ```
   ╔══════════════════════════════════════════╗
   ║      🌲  EdgeWake — Forest Guard  🌲     ║
   ╠══════════════════════════════════════════╣
   ║  Boot #1  |  Device: EDGEWAKE-001       ║
   ╚══════════════════════════════════════════╝
   [WAKE] Cause: COLD BOOT (power-on / reset)
   [MAIN] Cold boot or non-trigger wake. Going to sleep...
   💤 [SLEEP] Entering deep sleep... goodnight. 🌙
   ```
3. Hold a **lighter** near the flame sensor → the device should wake up!

---

## Part 2: n8n Workflow Setup

### 2.1 Import the Workflow

1. Open your n8n instance (cloud or self-hosted)
2. Click **"..."** → **Import from File**
3. Select `n8n_workflow/edgewake_workflow.json`
4. The entire workflow will appear on your canvas

### 2.2 Configure Credentials

#### Telegram Bot
1. Open Telegram → search for **@BotFather**
2. Send `/newbot` → follow the prompts
3. Copy the **API Token**
4. In n8n: Go to **Settings → Credentials → Add Credential → Telegram API**
5. Paste the token
6. Find your Chat ID: talk to **@userinfobot** on Telegram
7. Replace `YOUR_TELEGRAM_CHAT_ID` in **all** Telegram nodes

#### Google Drive (Optional)
1. In n8n: **Settings → Credentials → Add Credential → Google Drive OAuth2**
2. Follow the OAuth flow to authorize
3. Create a folder called `EdgeWake_Alerts` in your Google Drive
4. Update the folder reference in the Google Drive nodes

### 2.3 Activate the Workflow

1. Toggle the workflow to **Active** (top-right switch)
2. Copy the webhook URL (it will look like `https://your-n8n.app.n8n.cloud/webhook/edgewake`)
3. Paste it into `config.h` → `WEBHOOK_URL`
4. Re-upload the firmware

### 2.4 Test the Webhook

You can test with curl:
```bash
curl -X POST https://your-n8n.com/webhook/edgewake \
  -F "alert_type=Fire" \
  -F "device_id=EDGEWAKE-001" \
  -F "location=Forest Sector A" \
  -F "rms_energy=2500.0" \
  -F "image=@test_photo.jpg"
```

---

## Part 3: Dashboard

### 3.1 Open Locally

Simply open `dashboard/index.html` in any modern browser. No server needed.

### 3.2 Simulate an Alert

Click the **"Simulate Fire Alert"** button to see:
- The tier cascade light up sequentially
- Serial monitor output scrolling in real-time
- An alert card appearing in the feed
- Status indicators changing

This is perfect for **demo presentations** without needing the physical hardware.

---

## Part 4: Edge Impulse TinyML (Advanced — Optional)

To upgrade from the simple energy detector to a real ML classifier:

### 4.1 Collect Audio Data

1. Go to [edgeimpulse.com](https://edgeimpulse.com) → Create a project
2. Record audio samples:
   - **fire**: crackling fire sounds (~2 min)
   - **chainsaw**: chainsaw sounds (~2 min)
   - **background**: forest ambient noise (~2 min)
3. Label each sample with its class

### 4.2 Train the Model

1. Create an **Impulse**:
   - Input: Audio (MFE or MFCC)
   - Processing: Spectral features
   - Learning: Classification (Keras)
2. Train → aim for >90% accuracy
3. Go to **Deployment → Arduino Library**
4. Download the `.zip` library

### 4.3 Install in Arduino IDE

1. **Sketch → Include Library → Add .ZIP Library**
2. Select the downloaded Edge Impulse `.zip`
3. In `config.h`, set:
   ```cpp
   #define USE_TINYML_MODEL  1
   ```
4. In `audio_utils.h`, uncomment:
   ```cpp
   #include <Your_Edge_Impulse_inferencing.h>
   ```
5. Uncomment the TinyML inference code block
6. Re-upload

---

## Part 5: Live Demo Checklist

Use this checklist before your hackathon/demo:

- [ ] ESP32-CAM flashed with latest firmware
- [ ] `config.h` has correct WiFi + webhook URL
- [ ] Flame sensor sensitivity tuned (lighter at ~15cm triggers)
- [ ] n8n workflow is **Active**
- [ ] Telegram bot sends test messages
- [ ] Power bank fully charged
- [ ] Serial monitor visible on laptop (115200 baud)
- [ ] Dashboard open in browser for visual impact
- [ ] Lighter ready for trigger demo
- [ ] Phone ready to show Telegram notification

### Demo Script Timing

| Step | Duration | What Happens |
|:---|:---:|:---|
| Show serial "Deep Sleep..." | 5 sec | Prove zero-power state |
| Flick lighter near sensor | instant | Flame sensor triggers |
| Serial shows cascade | ~4 sec | Tiers 2→3→4→5 execute |
| Telegram notification arrives | ~3 sec | Photo + alert on phone |
| Serial shows "Deep Sleep..." | instant | Power-saving cycle complete |
| **Total demo time** | **~12 sec** | |

---

## File Structure

```
forest_gaurd/
├── firmware/
│   └── EdgeWake_Main/
│       ├── EdgeWake_Main.ino     ← Main sketch
│       ├── config.h              ← All configurable settings
│       ├── camera_utils.h        ← Camera init & capture
│       ├── audio_utils.h         ← Mic driver & audio analysis
│       └── network_utils.h       ← WiFi & HTTP upload
├── n8n_workflow/
│   └── edgewake_workflow.json    ← Import into n8n
├── dashboard/
│   ├── index.html                ← Monitoring dashboard
│   ├── style.css                 ← Dashboard styles
│   └── script.js                 ← Dashboard logic
├── docs/
│   ├── WIRING_GUIDE.md           ← Pin-by-pin wiring
│   └── SETUP_GUIDE.md           ← This file
└── master_file.txt               ← Original project spec
```

---

## FAQ

**Q: Can I use an MQ-2 smoke sensor instead of the flame sensor?**
A: Yes! The MQ-2's digital output pin works identically. Connect its DO pin to GPIO 13. You may need to adjust the sensitivity potentiometer.

**Q: The camera image is dark / purple.**
A: Enable PSRAM in Arduino IDE board settings. Without it, the camera falls back to a low-quality mode.

**Q: Can I add multiple sensor nodes?**
A: Yes. Change `DEVICE_ID` and `DEVICE_LOCATION` in `config.h` for each node. The n8n workflow will handle alerts from any device.

**Q: How long will the battery last?**
A: In deep sleep, the ESP32-CAM draws ~10µA. A 2000mAh 18650 cell can theoretically last **years** in sleep mode. Each wake-alert cycle uses ~160mA for ~5 seconds, which is negligible.

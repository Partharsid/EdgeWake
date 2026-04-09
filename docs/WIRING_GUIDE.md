# 🔌 EdgeWake — Wiring Guide

Complete wiring reference for the ESP32-CAM (AI-Thinker) based EdgeWake node.

---

## Board: ESP32-CAM (AI-Thinker)

```
                    ┌──────────────────┐
                    │    OV2640 Cam     │
                    │    ┌────────┐     │
                    │    │ CAMERA │     │
                    │    └────────┘     │
                    │                  │
              5V ── │ 5V          3V3 │ ── 3V3
             GND ── │ GND         IO16│
           IO12  ── │ IO12        IO0 │ ── (Boot)
           IO13  ── │ IO13        GND │ ── GND
           IO15  ── │ IO15        VCC │
           IO14  ── │ IO14        U0R │ ── FTDI TX
            IO2  ── │ IO2         U0T │ ── FTDI RX
            IO4  ── │ IO4 (Flash)     │
                    └──────────────────┘
```

---

## 1. Flame Sensor → ESP32-CAM

| Flame Sensor Pin | ESP32-CAM Pin | Notes |
|:---:|:---:|:---|
| **VCC** | **3V3** | Power (3.3V) |
| **GND** | **GND** | Common ground |
| **DO** (Digital Out) | **GPIO 13** | ⚠️ **Must be GPIO 13** — this is an RTC GPIO required for `ext0` deep-sleep wake-up |

> **Important:** GPIO 13 is `RTC_GPIO 14` internally. It's one of the few pins on the ESP32-CAM that supports hardware wake-up from deep sleep. Do **not** use any other pin.

### Flame Sensor Sensitivity
Most flame sensor modules have a potentiometer on the back. Adjust it so:
- The onboard LED turns **ON** when you hold a lighter ~15cm away
- The onboard LED turns **OFF** when there's no flame

---

## 2. INMP441 Microphone → ESP32-CAM

| INMP441 Pin | ESP32-CAM Pin | I2S Function |
|:---:|:---:|:---|
| **VDD** | **3V3** | Power (3.3V) |
| **GND** | **GND** | Common ground |
| **L/R** | **GND** | Selects LEFT channel (tie to GND) |
| **WS** | **GPIO 14** | Word Select (LRCK / Frame Sync) |
| **SCK** | **GPIO 15** | Serial Clock (BCLK / Bit Clock) |
| **SD** | **GPIO 12** | Serial Data Out (DOUT) |

> **Note:** These pins (12, 13, 14, 15) are normally used by the SD card slot on the ESP32-CAM. Since we are **not** using the SD card, they are free for the microphone and flame sensor.

### GPIO 12 Boot Warning
GPIO 12 is a strapping pin that controls flash voltage on boot. If the ESP32 fails to boot:
1. Disconnect the INMP441's SD line from GPIO 12
2. Upload the code
3. Reconnect GPIO 12 after upload

---

## 3. FTDI Programmer → ESP32-CAM (Upload Only)

| FTDI Pin | ESP32-CAM Pin |
|:---:|:---:|
| **TX** | **U0R** (RX) |
| **RX** | **U0T** (TX) |
| **GND** | **GND** |
| **VCC (5V)** | **5V** |

### Upload Mode
1. Connect **IO0** to **GND** with a jumper wire
2. Press the **RST** button (or power cycle)
3. Upload from Arduino IDE
4. **Remove** the IO0-GND jumper
5. Press **RST** again to run the code

---

## 4. Power Supply

### Option A: USB Power Bank (Demo/Testing)
- Connect a standard 5V USB power bank to the ESP32-CAM's **5V** and **GND** pins
- Simple and reliable for live demos

### Option B: 18650 Battery (Field Deployment)

| Component | Connection |
|:---|:---|
| 18650 Li-ion battery | → TP4056 B+/B- pads |
| TP4056 OUT+ | → ESP32-CAM **5V** (via boost converter if needed) |
| TP4056 OUT- | → ESP32-CAM **GND** |

> **Note:** The ESP32-CAM requires 5V input. A raw 18650 cell outputs ~3.7V. Use a TP4056 module paired with a small DC-DC boost converter (e.g., MT3608) to step up to 5V.

---

## Full Wiring Diagram (ASCII)

```
                    ┌─────────────┐
                    │  FLAME      │
                    │  SENSOR     │
                    │             │
                    │ VCC ── 3V3  │──────── ESP32-CAM 3V3
                    │ GND ── GND  │──────── ESP32-CAM GND
                    │ DO  ── DATA │──────── ESP32-CAM GPIO 13  (ext0 wake)
                    └─────────────┘

                    ┌─────────────┐
                    │  INMP441    │
                    │  MICROPHONE │
                    │             │
                    │ VDD ── 3V3  │──────── ESP32-CAM 3V3
                    │ GND ── GND  │──────── ESP32-CAM GND
                    │ L/R ── GND  │──────── ESP32-CAM GND  (LEFT channel)
                    │ WS  ── CLK  │──────── ESP32-CAM GPIO 14
                    │ SCK ── CLK  │──────── ESP32-CAM GPIO 15
                    │ SD  ── DATA │──────── ESP32-CAM GPIO 12
                    └─────────────┘

                    ┌─────────────┐
  5V Power Bank ───►│ ESP32-CAM   │
  or 18650+Boost    │ AI-Thinker  │
                    │             │
                    │  OV2640 cam │ (built-in, no wiring needed)
                    │  Flash LED  │ GPIO 4 (built-in)
                    └─────────────┘
```

---

## Pin Summary Table

| GPIO | Function | Module |
|:---:|:---|:---|
| **4** | Flash LED | Built-in (ESP32-CAM) |
| **12** | I2S SD (Data In) | INMP441 Microphone |
| **13** | ext0 Wake Interrupt | Flame Sensor DO |
| **14** | I2S WS (Word Select) | INMP441 Microphone |
| **15** | I2S SCK (Bit Clock) | INMP441 Microphone |
| **0** | Boot mode select | FTDI (IO0→GND for upload) |
| **U0R** | Serial RX | FTDI TX |
| **U0T** | Serial TX | FTDI RX |

---

## Troubleshooting

| Symptom | Fix |
|:---|:---|
| ESP32 won't boot | Disconnect GPIO 12, upload, reconnect |
| Camera shows no image | Check antenna, ensure PSRAM is enabled in Arduino IDE board settings |
| Mic always reads 0 | Check L/R pin is grounded; verify I2S port is `I2S_NUM_1` |
| Flame sensor never triggers | Adjust potentiometer sensitivity; test with lighter at ~10cm |
| Won't wake from deep sleep | Confirm flame sensor DO is on **GPIO 13** (not GPIO 12 or 14) |
| Brownout reset loop | Use 5V power source (not 3.3V); add 100µF capacitor on VCC |

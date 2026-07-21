# 🔌 EdgeWake — Simplified Wiring Guide

Complete wiring reference for the ESP32-CAM (AI-Thinker) based EdgeWake node.

---

## 1. Flame Sensor → ESP32-CAM

| Flame Sensor Pin | ESP32-CAM Pin | Notes |
|:---:|:---:|:---|
| **VCC** | **3V3** | Power (3.3V) |
| **GND** | **GND** | Common ground |
| **DO** (Digital Out) | **GPIO 13** | Used to wake the board from Deep Sleep. |

## 2. Vibration Sensor (SW-420) → ESP32-CAM

| Vibration Sensor Pin | ESP32-CAM Pin | Notes |
|:---:|:---:|:---|
| **VCC** | **3V3** | Power (3.3V) |
| **GND** | **GND** | Common ground |
| **DO** (Digital Out) | **GPIO 33** | We use GPIO 33 because it's completely safe (not a boot pin) and connects to the red LED! |

## 3. INMP441 Microphone → ESP32-CAM

| INMP441 Pin | ESP32-CAM Pin | I2S Function |
|:---:|:---:|:---|
| **VDD** | **3V3** | Power (3.3V) |
| **GND** | **GND** | Common ground |
| **L/R** | **GND** | Selects LEFT channel (tie to GND) |
| **WS** | **GPIO 14** | Word Select |
| **SCK** | **GPIO 15** | Serial Clock |
| **SD** | **GPIO 12** | Serial Data Out |

> **Note:** GPIO 12 is a special "boot pin". If your ESP32-CAM refuses to turn on, unplug GPIO 12, turn the board on, and plug it back in.

---

## Full Wiring Diagram (ASCII)

```text
                    ┌─────────────┐
                    │  FLAME      │
                    │  SENSOR     │
                    │ VCC ── 3V3  │──────── ESP32-CAM 3V3
                    │ GND ── GND  │──────── ESP32-CAM GND
                    │ DO  ── DATA │──────── ESP32-CAM GPIO 13
                    └─────────────┘

                    ┌─────────────┐
                    │  VIBRATION  │
                    │  SENSOR     │
                    │ VCC ── 3V3  │──────── ESP32-CAM 3V3 (share pin with Flame VCC)
                    │ GND ── GND  │──────── ESP32-CAM GND (share pin with Flame GND)
                    │ DO  ── DATA │──────── ESP32-CAM GPIO 33
                    └─────────────┘

                    ┌─────────────┐
                    │  INMP441    │
                    │  MICROPHONE │
                    │ VDD ── 3V3  │──────── ESP32-CAM 3V3
                    │ GND ── GND  │──────── ESP32-CAM GND
                    │ L/R ── GND  │──────── ESP32-CAM GND
                    │ WS  ── CLK  │──────── ESP32-CAM GPIO 14
                    │ SCK ── CLK  │──────── ESP32-CAM GPIO 15
                    │ SD  ── DATA │──────── ESP32-CAM GPIO 12
                    └─────────────┘
```

---

## Troubleshooting
- **Board doesn't upload code?** Make sure IO0 is connected to GND while uploading.
- **Board doesn't turn on?** Unplug GPIO 12, then restart the board.
- **Sensors too sensitive?** Turn the little blue potentiometer screw on the flame or vibration sensors.

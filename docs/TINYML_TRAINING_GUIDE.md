# EdgeWake — TinyML Audio Classification: Complete Training & Deployment Guide

> Step-by-step walkthrough to train a sound classification model on Edge Impulse
> and deploy it on the ESP32-CAM for real-time forest threat detection.

---

## Table of Contents

1. [Prerequisites & Tools](#1-prerequisites--tools)
2. [Phase 1 — Data Collection](#phase-1--data-collection)
3. [Phase 2 — Edge Impulse Project Setup](#phase-2--edge-impulse-project-setup)
4. [Phase 3 — Data Upload & Labeling](#phase-3--data-upload--labeling)
5. [Phase 4 — Impulse Design (Feature + Model)](#phase-4--impulse-design-feature--model)
6. [Phase 5 — Training the Model](#phase-5--training-the-model)
7. [Phase 6 — Testing & Validation](#phase-6--testing--validation)
8. [Phase 7 — Export as Arduino Library](#phase-7--export-as-arduino-library)
9. [Phase 8 — Integrate into EdgeWake Firmware](#phase-8--integrate-into-edgewake-firmware)
10. [Phase 9 — Flash & Field Test](#phase-9--flash--field-test)
11. [Troubleshooting](#troubleshooting)
12. [Appendix — Recommended Datasets](#appendix--recommended-datasets)

---

## 1. Prerequisites & Tools

### Hardware Required

| Item | Purpose |
|:---|:---|
| ESP32-CAM (AI-Thinker) | Target deployment board |
| INMP441 I2S MEMS Microphone | Audio input sensor |
| FTDI CP2102 USB-to-TTL Programmer | Firmware upload |
| Laptop/PC with USB port | Training + flashing |
| (Optional) Smartphone | Recording field audio samples |

### Software Required

| Software | Version | Purpose |
|:---|:---|:---|
| [Edge Impulse Studio](https://studio.edgeimpulse.com) | Free account | Model training platform |
| [Arduino IDE](https://www.arduino.cc/en/software) | 2.x | Firmware compilation + upload |
| [Audacity](https://www.audacityteam.org/) | 3.x | Audio recording, trimming, cleaning |
| [Edge Impulse CLI](https://docs.edgeimpulse.com/docs/tools/edge-impulse-cli) | Latest | (Optional) Direct data upload from device |
| ESP32 Board Support | Via Arduino Boards Manager | ESP32-CAM compilation |

### Install Edge Impulse CLI (Optional but recommended)

```bash
# Requires Node.js 18+ installed
npm install -g edge-impulse-cli

# Verify installation
edge-impulse-daemon --version
```

### Arduino IDE — Install ESP32 Board Support

1. Open Arduino IDE → **File → Preferences**
2. In "Additional Boards Manager URLs", add:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
3. Go to **Tools → Board → Boards Manager** → Search "esp32" → Install **esp32 by Espressif Systems**
4. Select Board: **AI Thinker ESP32-CAM**

---

## Phase 1 — Data Collection

> **This is the MOST important step.** A model is only as good as its training data.
> Budget 60–70% of your total effort here.

### 1.1 Define Your Classes

For EdgeWake, we need **3 classes**:

| Class | Label | Description |
|:---|:---|:---|
| 🔥 Fire | `fire` | Crackling, roaring, popping sounds of fire |
| 🪓 Chainsaw | `chainsaw` | Motorized chainsaw / cutting wood sounds |
| 🌿 Background | `background` | Normal forest ambience — wind, birds, rain, silence |

### 1.2 How Much Data Do You Need?

| Class | Minimum | Recommended | Ideal |
|:---|:---:|:---:|:---:|
| `fire` | 3 minutes | 8–10 minutes | 15+ minutes |
| `chainsaw` | 3 minutes | 8–10 minutes | 15+ minutes |
| `background` | 5 minutes | 15–20 minutes | 30+ minutes |

> **Why more background data?** Because in real deployment, 99% of the time the
> forest sounds are "background." The model needs to be extremely confident about
> what is NOT a threat.

### 1.3 Where to Get Audio Data

#### Option A: Download from Free Sound Libraries (Fast Start)

| Source | URL | Notes |
|:---|:---|:---|
| **ESC-50 Dataset** | https://github.com/karolpiczak/ESC-50 | Has "crackling_fire", "chainsaw" classes |
| **UrbanSound8K** | https://urbansounddataset.weebly.com/ | Has chainsaw sounds |
| **Freesound.org** | https://freesound.org | Search "fire crackling", "chainsaw", "forest ambience" |
| **BBC Sound Effects** | https://sound-effects.bbcrewind.co.uk/ | High quality, free for personal use |
| **Kaggle Audio Datasets** | https://www.kaggle.com/datasets | Search "environmental sound classification" |

#### Option B: Record Your Own Data (Best Quality)

**For fire sounds:**
```
1. Light a controlled campfire or use a lighter near the mic
2. Record using Audacity or your phone
3. Settings: 16 kHz sample rate, Mono, 16-bit WAV
4. Record at different distances: 0.5m, 1m, 3m, 5m
5. Include variations: small flame, large fire, crackling wood
```

**For chainsaw sounds:**
```
1. Record an actual chainsaw if accessible
2. Or record from YouTube videos played through a speaker
   (not ideal, but acceptable for a hackathon demo)
3. Include: idle, revving, cutting phases
```

**For background / forest sounds:**
```
1. Go outdoors to a park/garden/forest
2. Record 20-30 minutes of ambient sound
3. Include: wind, birds, vehicles in distance, silence, rain
4. Record at different times of day
5. THIS DATA IS THE MOST IMPORTANT — variety matters!
```

### 1.4 Prepare Audio Files

Use **Audacity** to standardize all recordings:

```
1. Open the audio file in Audacity
2. Convert to Mono:  Tracks → Mix → Mix Stereo Down to Mono
3. Set sample rate:  Tracks → Resample → 16000 Hz
4. Trim silence and dead spots
5. Normalize volume:  Effect → Normalize → -1.0 dB
6. Export as WAV:  File → Export Audio → WAV (16-bit PCM)
```

### 1.5 Organize Your Files

```
training_data/
├── fire/
│   ├── fire_campfire_01.wav
│   ├── fire_crackling_02.wav
│   ├── fire_lighter_03.wav
│   └── ... (aim for 15-30 clips, 5-30 sec each)
├── chainsaw/
│   ├── chainsaw_cutting_01.wav
│   ├── chainsaw_idle_02.wav
│   └── ... (aim for 15-30 clips)
└── background/
    ├── bg_forest_morning_01.wav
    ├── bg_wind_rain_02.wav
    ├── bg_birds_03.wav
    ├── bg_silence_04.wav
    └── ... (aim for 30-50 clips)
```

---

## Phase 2 — Edge Impulse Project Setup

### 2.1 Create an Account

1. Go to [https://studio.edgeimpulse.com](https://studio.edgeimpulse.com)
2. Click **Sign Up** (free for developers)
3. Verify your email

### 2.2 Create a New Project

1. Click **"+ Create new project"**
2. Project name: `EdgeWake-Forest-Audio`
3. Select project type: **Audio**
4. Target device: Choose **ESP32** (or "Other" if ESP32 isn't listed)
5. Click **Create**

### 2.3 Configure Project Settings

1. Go to **Dashboard → Project info**
2. Set "Labeling method" to: **One label per data item**
3. Set target device latency: **< 500ms**
4. Set target device RAM: **4096 KB** (PSRAM)

---

## Phase 3 — Data Upload & Labeling

### Method A: Upload via Web UI (Easiest)

1. In Edge Impulse Studio, click **"Data acquisition"** in the left menu
2. Click **"+ Add data"** → **"Upload data"**
3. Settings:
   - Upload mode: **Select a file**
   - Category: **Split automatically between training and testing** (80/20)
   - Label: Enter the class name (e.g., `fire`)
4. Select all `.wav` files from your `fire/` folder
5. Click **Upload**
6. Repeat for `chainsaw/` and `background/` folders with appropriate labels

### Method B: Upload via CLI (Faster for Bulk)

```bash
# Login to your Edge Impulse account
edge-impulse-uploader --clean

# Upload each class
edge-impulse-uploader --category training --label fire training_data/fire/*.wav
edge-impulse-uploader --category training --label chainsaw training_data/chainsaw/*.wav
edge-impulse-uploader --category training --label background training_data/background/*.wav
```

### 3.1 Verify Your Dataset

After uploading, on the **Data acquisition** page you should see:

```
Training set:     ~80% of total samples
Testing set:      ~20% of total samples
Classes:          fire, chainsaw, background
Total duration:   30-60+ minutes recommended
```

> ⚠️ **Check for class imbalance!** If `background` has 20 minutes but `fire` has
> only 2 minutes, the model will be biased. Try to keep classes roughly balanced
> (or at most 3:1 ratio).

---

## Phase 4 — Impulse Design (Feature + Model)

> An "Impulse" in Edge Impulse = Processing Block + Learning Block.
> This is where you define HOW audio is turned into features and WHAT model
> architecture to use.

### 4.1 Create the Impulse

1. Click **"Create impulse"** in the left menu
2. Configure the **Time series data** block:
   - Window size: **2000 ms** (2 seconds — matches `AUDIO_SAMPLE_DURATION` in config.h)
   - Window increase: **500 ms** (sliding window for more training samples)
   - Frequency: **16000 Hz** (matches `AUDIO_SAMPLE_RATE`)
   - Zero-pad data: **Enabled**
3. Add a **Processing block**: Click **"+ Add a processing block"**
   - Choose: **Audio (MFCC)** ← RECOMMENDED for environmental sounds
   - (Alternative: Audio (MFE) — slightly different; MFCC is better for this use case)
4. Add a **Learning block**: Click **"+ Add a learning block"**
   - Choose: **Classification** (not anomaly detection)
5. Click **"Save Impulse"**

### 4.2 Configure MFCC Parameters

1. Click **"MFCC"** in the left menu
2. Use these recommended settings:

| Parameter | Value | Why |
|:---|:---:|:---|
| Number of coefficients | **13** | Standard for audio classification |
| Frame length | **0.04** (40 ms) | Good time resolution |
| Frame stride | **0.02** (20 ms) | 50% overlap — more features |
| Filter number | **40** | Mel filterbank size (default is good) |
| FFT length | **512** | Matches 16 kHz well |
| Low frequency | **0** Hz | Capture full spectrum |
| High frequency | **8000** Hz | Nyquist of 16 kHz |
| Noise floor (dB) | **-52** | Default — helps with quiet sounds |

3. Click **"Save parameters"**
4. Click **"Generate features"**
   - Wait for the feature extraction job to complete
   - You should see a **Feature explorer** scatter plot showing separation between classes
   - **Good sign:** Clusters of different colors are well-separated
   - **Bad sign:** All dots are mixed together → need more/better data

### 4.3 Evaluate Feature Quality

On the Feature explorer:
- `fire` dots (red) should cluster together, separate from others
- `chainsaw` dots (blue) should form their own cluster
- `background` dots (green) should be in a different region

If the classes overlap heavily, consider:
- Adding more diverse training data
- Trying MFE instead of MFCC
- Adjusting the window size

---

## Phase 5 — Training the Model

### 5.1 Configure the Neural Network

1. Click **"Classifier"** in the left menu
2. Set up the architecture:

**Recommended architecture for ESP32 (small + accurate):**

| Layer | Setting |
|:---|:---|
| Input features | Auto (from MFCC) |
| Dense Layer 1 | **64 neurons**, ReLU activation |
| Dropout | **0.25** (prevents overfitting) |
| Dense Layer 2 | **32 neurons**, ReLU activation |
| Dropout | **0.25** |
| Output Layer | **3 neurons** (fire, chainsaw, background), Softmax |

3. Training settings:
   - **Number of training cycles (epochs):** 100–200
   - **Learning rate:** 0.005 (default)
   - **Minimum confidence rating:** 0.6 (60%)
   - **Auto-balance dataset:** ✅ Enable this
   - **Data augmentation:** ✅ Enable this (Edge Impulse will add noise, shift pitch)

4. Click **"Start training"** → Wait 2-10 minutes

### 5.2 Read the Training Results

After training completes, you'll see:

```
ACCURACY:          XX.X%      ← Target: > 85%
LOSS:              X.XXX      ← Target: < 0.5
CONFUSION MATRIX:  (shows per-class accuracy)
```

**Interpreting the Confusion Matrix:**

```
              Predicted →
              fire    chainsaw   background
Actual ↓
fire         [92%]      3%         5%        ← 92% correct for fire
chainsaw      2%       [89%]       9%        ← 89% correct for chainsaw  
background    4%        1%        [95%]      ← 95% correct for background
```

**What's acceptable:**

| Metric | Bad | Okay | Good | Great |
|:---|:---:|:---:|:---:|:---:|
| Overall Accuracy | < 75% | 75–85% | 85–92% | > 92% |
| Per-class Accuracy | < 70% | 70–80% | 80–90% | > 90% |
| Loss | > 1.0 | 0.5–1.0 | 0.2–0.5 | < 0.2 |

### 5.3 If Accuracy Is Too Low

Try these in order:
1. **Add more data** — especially for the weak class
2. **Increase epochs** to 200–300
3. **Try a 1D CNN architecture** instead of Dense NN:
   - Conv1D (8 filters, kernel 3) → MaxPool1D → Conv1D (16 filters) → Dense(32) → Output
4. **Adjust MFCC parameters** — try more coefficients (20 instead of 13)
5. **Remove noisy/mislabeled samples** from the dataset

---

## Phase 6 — Testing & Validation

### 6.1 Model Testing (Automatic)

1. Click **"Model testing"** in the left menu
2. Click **"Classify all"**
3. This runs the model against the 20% holdout test set
4. Review overall accuracy and per-class breakdown

### 6.2 Live Classification (Optional — with device)

If you have an INMP441 wired to your ESP32:

```bash
# Connect your ESP32 and run:
edge-impulse-daemon

# Follow the prompts to select your project
# Then in Edge Impulse Studio → "Live classification" → Start sampling
```

### 6.3 Check Model Size & Performance

Go to **Dashboard** and check the **On-device performance** panel:

| Metric | Target for ESP32 | Your value |
|:---|:---:|:---:|
| Inferencing time | < 500 ms | ___ ms |
| Peak RAM usage | < 100 KB | ___ KB |
| Flash usage (model) | < 500 KB | ___ KB |

> If model is too large, switch from CNN to Dense NN, or reduce the number of
> MFCC coefficients from 13 to 10.

---

## Phase 7 — Export as Arduino Library

### 7.1 Deploy the Model

1. Click **"Deployment"** in the left menu
2. Search for: **Arduino library**
3. Optimization: Select **Quantized (int8)** ← IMPORTANT for ESP32
4. Click **"Build"**
5. A `.zip` file will download (e.g., `EdgeWake-Forest-Audio_inferencing.zip`)

### 7.2 Install in Arduino IDE

1. Open Arduino IDE
2. Go to **Sketch → Include Library → Add .ZIP Library...**
3. Select the downloaded `.zip` file
4. Arduino will install it → You'll see a confirmation message

### 7.3 Verify Installation

1. Go to **File → Examples** → Scroll down to find your library name
   (e.g., `EdgeWake-Forest-Audio_inferencing`)
2. You should see example sketches — this means installation was successful

The library adds these key files:
```
Arduino/libraries/EdgeWake-Forest-Audio_inferencing/
├── src/
│   ├── edge-impulse-sdk/           # TFLite Micro runtime
│   ├── model-parameters/           # Your trained model weights
│   └── EdgeWake-Forest-Audio_inferencing.h   # Main include header
```

---

## Phase 8 — Integrate into EdgeWake Firmware

### 8.1 Update `config.h`

Change the TinyML toggle from `0` to `1`:

```cpp
// --- Edge Impulse TinyML toggle ---
// Set to 1 when you have a trained Edge Impulse model library
// installed.  Set to 0 to fall back to the simple energy detector.
#define USE_TINYML_MODEL  1     // ← CHANGE THIS FROM 0 TO 1
```

### 8.2 Update `audio_utils.h`

**Step 1:** Uncomment the Edge Impulse include at the top of the file:

```cpp
// Change this line (around line 20-21):
// #include <Your_Edge_Impulse_inferencing.h>

// To this (use your actual library name):
#include <EdgeWake-Forest-Audio_inferencing.h>
```

**Step 2:** Uncomment the inference code inside `analyseAudioTinyML()` function.
Replace the placeholder block (lines ~162-209) with:

```cpp
#if USE_TINYML_MODEL
bool analyseAudioTinyML(const int16_t *buffer, int numSamples) {
  Serial.println("[ML] Running TinyML inference...");

  // 1. Create a signal from the audio buffer
  signal_t signal;
  int err = numpy::signal_from_buffer((int16_t *)buffer, numSamples, &signal);
  if (err != 0) {
    Serial.println("[ML] Signal creation failed");
    return analyseAudioSimple(buffer, numSamples);  // fallback
  }

  // 2. Run the classifier
  ei_impulse_result_t result = { 0 };
  err = run_classifier(&signal, &result, false /* debug */);
  if (err != EI_IMPULSE_OK) {
    Serial.printf("[ML] Classifier failed (%d)\n", err);
    return analyseAudioSimple(buffer, numSamples);  // fallback
  }

  // 3. Print all class scores
  float fireScore     = 0.0;
  float chainsawScore = 0.0;

  Serial.println("[ML] Classification results:");
  for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
    Serial.printf("[ML]   %s: %.4f\n",
                  result.classification[ix].label,
                  result.classification[ix].value);

    if (strcmp(result.classification[ix].label, "fire") == 0) {
      fireScore = result.classification[ix].value;
    }
    if (strcmp(result.classification[ix].label, "chainsaw") == 0) {
      chainsawScore = result.classification[ix].value;
    }
  }

  // 4. Threat detected if fire OR chainsaw confidence > 60%
  bool threat = (fireScore > 0.6 || chainsawScore > 0.6);

  if (threat) {
    Serial.printf("[ML] ⚠ THREAT detected! fire=%.2f chainsaw=%.2f\n",
                  fireScore, chainsawScore);
  } else {
    Serial.println("[ML] ✓ No threat — background sound.");
  }

  return threat;
}
#endif
```

### 8.3 Compile & Verify

1. Open `EdgeWake_Main.ino` in Arduino IDE
2. Board: **AI Thinker ESP32-CAM**
3. PSRAM: **Enabled**
4. Partition Scheme: **Huge APP (3MB No OTA/1MB SPIFFS)** ← IMPORTANT for model fit
5. Click **Verify (✓)** to compile without uploading

Expected output:
```
Sketch uses XXXXX bytes (XX%) of program storage space.
Global variables use XXXXX bytes (XX%) of dynamic memory.
```

> If you get "Sketch too large" errors, change Partition Scheme to
> "Huge APP (3MB No OTA/1MB SPIFFS)"

### 8.4 Flash to ESP32-CAM

1. Wire FTDI to ESP32-CAM:
   ```
   FTDI GND  → ESP32-CAM GND
   FTDI 5V   → ESP32-CAM 5V
   FTDI TX   → ESP32-CAM U0R
   FTDI RX   → ESP32-CAM U0T
   ESP32 IO0 → GND  (hold during flash only)
   ```
2. Click **Upload (→)**
3. After upload completes, **disconnect IO0 from GND**
4. Press **RST** button on ESP32-CAM

---

## Phase 9 — Flash & Field Test

### 9.1 Serial Monitor Test

1. Open **Tools → Serial Monitor** at **115200 baud**
2. Press RST → You should see:
   ```
   [BOOT] EdgeWake v2.0 — TinyML ENABLED
   [BOOT] Wake cause: External (ext1)
   [MIC] Initialising I2S...
   [MIC] Initialised OK.
   [MIC] Recording audio...
   [MIC] Recorded 32000 samples (2.0 sec)
   [ML] Running TinyML inference...
   [ML] Classification results:
   [ML]   background: 0.8912
   [ML]   chainsaw: 0.0543
   [ML]   fire: 0.0545
   [ML] ✓ No threat — background sound.
   [AUDIO] ✗ False alarm — no threat signature found.
   [MAIN] Entering deep sleep...
   ```

### 9.2 Test with Real Sounds

**Fire test:**
1. Use a lighter near the mic  
2. Or play fire crackling from your phone speaker  
3. Expected: `fire: 0.75+` → threat detected  

**Chainsaw test:**
1. Play chainsaw audio from phone speaker near mic  
2. Expected: `chainsaw: 0.70+` → threat detected  

**Background test:**
1. Normal room / outdoor silence  
2. Expected: `background: 0.85+` → false alarm, back to sleep  

### 9.3 Tune the Confidence Threshold

If the model triggers on background noise too often, increase the threshold in
`audio_utils.h`:

```cpp
// Change from 0.6 to 0.7 or 0.8:
bool threat = (fireScore > 0.7 || chainsawScore > 0.7);
```

If the model misses real threats, lower it:

```cpp
bool threat = (fireScore > 0.5 || chainsawScore > 0.5);
```

---

## Troubleshooting

### Compilation Errors

| Error | Fix |
|:---|:---|
| `fatal error: EdgeWake-Forest-Audio_inferencing.h: No such file` | Re-install the ZIP library. Check the exact header name in `/libraries/` folder. |
| `Sketch too large` | Change Partition Scheme to **Huge APP (3MB No OTA)** |
| `PSRAM not found` | Enable PSRAM in Tools menu. Check board selection. |
| `region 'dram0_0_seg' overflowed` | Model too large. Retrain with fewer neurons or switch to Dense NN. |

### Runtime Errors

| Issue | Fix |
|:---|:---|
| Inference always says `background` | Model undertrained. Add more fire/chainsaw samples and retrain. |
| Inference always says `fire` | Class imbalance. Add more background data. Check audio normalization. |
| Inference takes > 1 second | Model too complex. Use Dense NN instead of CNN. Reduce MFCC coefficients. |
| ESP32 crashes during inference | Memory overflow. Ensure PSRAM is enabled. Reduce model size. |

---

## Appendix — Recommended Datasets

### Free Audio Datasets for Environmental Sound Classification

| Dataset | Classes | Size | URL |
|:---|:---|:---|:---|
| **ESC-50** | 50 classes incl. fire, chainsaw | 2,000 clips | [GitHub](https://github.com/karolpiczak/ESC-50) |
| **ESC-10** | 10 classes (subset of ESC-50) | 400 clips | [GitHub](https://github.com/karolpiczak/ESC-50) |
| **UrbanSound8K** | 10 urban classes | 8,732 clips | [Website](https://urbansounddataset.weebly.com/) |
| **AudioSet** | 632 classes | 2M+ clips | [Google Research](https://research.google.com/audioset/) |
| **FSD50K** | 200 classes | 51,197 clips | [Zenodo](https://zenodo.org/record/4060432) |

### What to Download from ESC-50 for EdgeWake

```
ESC-50 class mapping:
  Class 12: "crackling_fire"  → Use for "fire" label
  Class 38: "chainsaw"        → Use for "chainsaw" label
  Classes 0-4: Nature sounds  → Use for "background" label
    (rain, sea_waves, crackling_fire excluded, wind, etc.)
```

---

## Summary — The Complete Pipeline

```
┌──────────────┐     ┌──────────────┐     ┌──────────────┐     ┌──────────────┐
│  1. COLLECT   │     │  2. UPLOAD   │     │  3. DESIGN   │     │  4. TRAIN    │
│  Audio Data   │────▶│  to Edge     │────▶│  MFCC +      │────▶│  Dense NN    │
│  3 classes    │     │  Impulse     │     │  Impulse     │     │  64→32→3     │
└──────────────┘     └──────────────┘     └──────────────┘     └──────────────┘
                                                                       │
┌──────────────┐     ┌──────────────┐     ┌──────────────┐            │
│  7. FIELD    │     │  6. FLASH    │     │  5. EXPORT   │◀───────────┘
│  TEST &      │◀────│  to ESP32    │◀────│  Arduino     │
│  TUNE        │     │  -CAM        │     │  Library     │
└──────────────┘     └──────────────┘     └──────────────┘
```

**Total estimated time:** 4–8 hours (most time spent on data collection)

---

*Document version: 1.0 — EdgeWake Project*
*Last updated: April 2026*

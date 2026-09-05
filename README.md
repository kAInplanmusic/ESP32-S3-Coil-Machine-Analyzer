# ESP32-S3 Coil Machine Analyzer - Advanced Multi-Point Waveform Analysis

**Version 3.0** - Advanced embedded oscilloscope system for magnetic coil machine impact analysis

## Overview

A comprehensive ESP32-S3 based system for real-time analysis of coil striking machines with:
- **Multi-point waveform extraction** (up to 20 measurement points per cycle)
- **Harmonic analysis** (Peak ratios, THD calculation)
- **Decay rate measurement** (Exponential damping analysis)
- **CPS measurement** (0-250 cycles per second)
- **Quality scoring** (Consistency, harmonic content, signal strength)
- **Real-time visualization** with interactive UI views

## Hardware Requirements

### Main Controller
- **ESP32-S3 DevKit** (Dual-core, 240MHz, 8MB PSRAM)
- 3.3V LDO regulator (built-in)

### Display
- **3.5" TFT Display** (480x320 pixels)
  - ILI9488 or ST7796 driver
  - 16-bit color (RGB565)
  - SPI interface
- **Backlight** PWM controlled (adjustable brightness)

### Audio Input
- **I2S Microphone Module** (44.1 kHz, 16-bit mono)
  - MAX9814 (with automatic gain control) OR
  - INMP441 (digital MEMS microphone)
- Microphone capsule (omnidirectional, -30dB@1kHz)

### Optional Sensors
- **MPU6050** Accelerometer (for vibration analysis)
- **SD Card Module** (for data logging)

### Power Supply
- 5V / 2A USB power or external PSU
- Optional: 18650 battery + charging module for portable operation

## Pin Configuration

### ESP32-S3 GPIO
```
I2S Audio Input:
  GPIO 3  (U0RXD) -> I2S_SCK   (Serial Clock)
  GPIO 4  (U0TXD) -> I2S_WS    (Word Select)
  GPIO 2  (RXD2)  -> I2S_SD    (Serial Data)

TFT Display (SPI):
  GPIO 5  (CS)    -> TFT_CS    (Chip Select)
  GPIO 6  (DC)    -> TFT_DC    (Data/Command)
  GPIO 7  (RST)   -> TFT_RST   (Reset)
  GPIO 8  (PWM)   -> TFT_BL    (Backlight)
  GPIO 11 (MOSI)  -> TFT_MOSI  (SPI Data Out)
  GPIO 12 (SCK)   -> TFT_SCLK  (SPI Clock)
  GPIO 13 (MISO)  -> TFT_MISO  (SPI Data In)
```

## Measurement Specifications

### Audio Processing
- **Sample Rate**: 44.1 kHz (Nyquist limit: 22 kHz)
- **Bit Depth**: 16-bit signed integer
- **FFT Size**: 2048 points (46.4 ms window)
- **Frequency Resolution**: 21.5 Hz per bin
- **Processing Latency**: <50 ms

### CPS Measurement
- **Range**: 0-250 cycles per second
- **Resolution**: 0.01 CPS
- **Measurement Window**: 100 complete impact cycles
- **Accuracy**: ±2% (dependent on microphone positioning)

### Waveform Analysis
- **Measurement Points**: Up to 20 per cycle (configurable)
- **Point Types**:
  1. Baseline (pre-impulse reference)
  2. Rising Edge (impulse start, C-point)
  3. Peak 1 (first maximum)
  4. Valley 1 (first minimum)
  5. Peak 2 (harmonic peak)
  6. Valley 2 (harmonic valley)
  7-20. Additional harmonics (up to 8 more peaks/valleys)

### Quality Metrics
- **Consistency**: Jitter analysis across all measurement points (0-100%)
- **Harmonic Ratios**: H2/H1, H3/H1 amplitude comparisons
- **Decay Rate**: Exponential damping coefficient
- **Q-Factor**: Oscillation quality (Peak1 vs Peak2 damping)
- **THD**: Total harmonic distortion percentage

## Software Architecture

### Core Components

#### 1. **I2SAudio** (`i2s_audio.h/cpp`)
Handles microphone audio capture via I2S protocol
- DMA buffering (2 buffers × 256 samples)
- Automatic gain control
- Reference calibration

#### 2. **SignalProcessor** (`signal_processor.h/cpp`)
General signal analysis and FFT computation
- FFT with windowing (Hann/Blackman-Harris)
- Peak detection
- CPS calculation from frequency spectrum

#### 3. **AdvancedImpactAnalyzer** (`advanced_impact_analyzer.h/cpp`)
Multi-point waveform extraction and analysis
- Automatic extrema detection (peaks/valleys)
- Slope angle calculation
- Harmonic ratio computation
- Decay rate analysis
- Jitter measurement per point
- Quality scoring

#### 4. **DisplayManager** (`display_manager.h/cpp`)
TFT display abstraction layer
- Graphics primitives (lines, rectangles, circles)
- Font rendering
- Color management

#### 5. **AdvancedMeasurementUI** (`advanced_measurement_ui.h/cpp`)
Interactive multi-view measurement interface
- **STATE_IDLE**: Welcome screen
- **STATE_MEASURING**: Real-time progress
- **STATE_WAVEFORM_LIVE**: Waveform graph with point markers
- **STATE_POINTS_TABLE**: Measurement point coordinates
- **STATE_HARMONICS**: Harmonic amplitude bar chart
- **STATE_DECAY_ANALYSIS**: Decay curve visualization
- **STATE_QUALITY_BREAKDOWN**: Quality metric bars

### Task Architecture (FreeRTOS)
```
Core 0 (240MHz):
  ├─ audioCaptureTask (Priority 3, HIGH)
  │  └─ I2S audio input, sample buffering
  └─ watchdog/idle

Core 1 (240MHz):
  ├─ measurementTask (Priority 2, MEDIUM)
  │  └─ CPS calculation, statistics
  └─ displayUpdateTask (Priority 1, LOW)
     └─ UI rendering (~100ms refresh)
```

## Measurement Process

### Step 1: Calibration
```
1. Press START
2. Microphone gain auto-calibrated (1-second baseline)
3. Reference noise floor detected
```

### Step 2: Collection Phase
```
For each impact cycle:
  1. Rising edge detection (zero crossing)
  2. Waveform capture (~100ms per cycle)
  3. Extrema detection (peaks, valleys)
  4. Slope angle calculation
  5. Energy per segment calculation
  6. Store measurement point data

  Repeat until 100 cycles collected
```

### Step 3: Analysis Phase
```
Statistics computed from all 100 waveforms:
  - Period/CPS (mean and jitter)
  - Point timing consistency (std dev)
  - Harmonic ratios (mean and variance)
  - Decay rates (mean and variance)
  - Overall quality score
```

### Step 4: Display Results
```
- Color-coded quality indicator
- Waveform visualization with point overlay
- Point coordinate table
- Harmonic bar chart
- Decay curve
- Quality breakdown bars
```

## Quality Scoring System

### Score Breakdown (0-100%)

| Metric | Weight | Calculation |
|--------|--------|-------------|
| **Consistency** | 60% | Jitter across all points vs 0.5ms threshold |
| **Harmonics** | 40% | Distance from ideal 2:1 peak ratio |
| **Signal Strength** | (in consistency) | Peak amplitude vs 0.3 threshold |

### Color Coding
```
95-100% : BRIGHT GREEN   (Perfect)
86-94%  : GREEN          (Excellent)
75-85%  : YELLOW         (Good)
61-74%  : ORANGE         (Fair)
0-60%   : RED            (Poor)
```

## Usage

### Hardware Setup

1. **Connect Microphone**
   - I2S_SCK → GPIO 3
   - I2S_WS → GPIO 4
   - I2S_SD → GPIO 2
   - GND → GND
   - VDD → 3.3V

2. **Connect TFT Display**
   - TFT_CS → GPIO 5
   - TFT_DC → GPIO 6
   - TFT_RST → GPIO 7
   - TFT_MOSI → GPIO 11
   - TFT_SCLK → GPIO 12
   - TFT_MISO → GPIO 13
   - TFT_BL → GPIO 8 (with 330Ω resistor to 3.3V)

3. **Power**
   - Connect 5V USB to ESP32-S3

### Software Setup

```bash
# 1. Clone repository
git clone https://github.com/kAInplanmusic/ESP32-S3-Coil-Machine-Analyzer.git
cd ESP32-S3-Coil-Machine-Analyzer

# 2. Install PlatformIO
pip install platformio

# 3. Build and upload
pio run -t upload -e esp32-s3-devkitc-1

# 4. Monitor serial output
pio device monitor -b 115200
```

### Operation

**On Display:**
- Press **START** button to begin measurement
- Use arrow buttons to navigate between views
- Press **START** again on results to reset

**Via Serial Console:**
```
Command      Description
-------------------------------------------
s            START new measurement
r            RESET analyzer
c            CALIBRATE microphone
+            Increase gain (+5dB)
-            Decrease gain (-5dB)
i            Print detailed statistics
d            Set microphone distance
a            Set microphone angle
h            Help/command list
```

## Configuration

Edit `include/config.h` to adjust:

```cpp
// Audio
#define SAMPLE_RATE              44100  // Hz
#define FFT_SIZE                 2048   // Points

// CPS Range
#define CPS_MIN                  0.0f   // Hz
#define CPS_MAX                  250.0f // Hz

// Measurement
#define IMPACT_MEASURE_COUNT     100    // Cycles
#define JITTER_MAX_ACCEPTABLE    0.5f   // ms

// Microphone Position
#define MICROPHONE_DISTANCE_MM   100.0f // mm from coil
#define MICROPHONE_ANGLE_DEG     0.0f   // degrees from vertical

// Quality Weighting
#define QUALITY_CONSISTENCY_WEIGHT 0.6f // 60%
#define QUALITY_SIGNAL_WEIGHT      0.4f // 40%
```

## Advanced Features

### Harmonic Analysis

The system automatically detects and measures harmonic content:
- **Peak 1** (Fundamental): Primary impact frequency
- **Peak 2** (1st Harmonic): Typically 0.3-0.6 × Peak1
- **Peak 3** (2nd Harmonic): Typically 0.1-0.3 × Peak1
- **THD**: √(H2² + H3²) / Fundamental × 100%

### Decay Rate Analysis

Exponential decay model: A(t) = A₀ × e^(-decay_rate × t)

- **Q-Factor**: π / decay_rate (higher = better oscillation)
- **Expected Q**: 2-8 for good strikes (5-20ms decay)

### Jitter Measurement

For each measurement point:
- Timing jitter = σ(point_time) across 100 cycles
- Amplitude jitter = σ(point_amplitude) across 100 cycles
- Total consistency = 100% - (jitter / threshold) × 100%

## Performance Metrics

### System Resources
- **CPU Usage**: ~60% Core 0 (audio), ~30% Core 1 (processing)
- **Memory**: ~200KB static + 100KB dynamic (per 100 cycles)
- **Flash**: ~500KB program, ~256KB SPIFFS
- **Update Rate**: 10 Hz UI refresh

### Measurement Accuracy
- **CPS**: ±2% (0-250 range)
- **Harmonics**: ±5% amplitude ratio
- **Decay**: ±3% Q-factor
- **Consistency**: ±1% (σ < 0.1ms)

## Troubleshooting

### No Audio Input
1. Check I2S pins and connections
2. Verify I2S clock: `Serial.print("I2S running")` check
3. Run calibration: `c` command in serial console
4. Check microphone power supply (3.3V, GND)

### Low Quality Score
1. Verify microphone distance/angle
2. Check for ambient noise
3. Ensure proper microphone alignment
4. Run gain adjustment: `+` or `-` command

### Display Not Showing
1. Check SPI pins and 3.3V supply
2. Verify backlight connected to GPIO 8
3. Reset display: Check RST pin (GPIO 7)
4. Try different rotation setting in config.h

### Inconsistent CPS Readings
1. Check microphone positioning consistency
2. Verify impact force is uniform
3. Reduce ambient noise
4. Run multiple measurements for averaging

## Data Logging (Optional)

For SD card logging, uncomment in config.h:
```cpp
#define LOG_SD_CARD true
```

Logs CSV format:
```
Cycle,Time_ms,Amplitude,dB,Energy,Slope_deg
1,0.5,0.45,-6.9,0.12,45.2
2,0.8,0.42,-7.5,0.11,42.8
...
```

## References

### Signal Processing
- Welch's method for PSD estimation
- FFT via KISSFFT library
- Butterworth bandpass filtering (100Hz-8kHz)

### Hardware
- ESP32-S3 Technical Reference Manual
- ILI9488 Display Driver Datasheet
- INMP441 MEMS Microphone Datasheet

## License

MIT License - Feel free to modify and distribute

## Author

Developed for tattoo coil machine impact analysis and quality control.

---

**Last Updated**: 2026-09-05
**Version**: 3.0 Advanced Multi-Point Waveform Analysis

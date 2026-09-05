# ESP32-S3 Coil Machine Analyzer

**Real-time oscilloscope and signal analysis system for electromagnetic coil machine monitoring**

## 🎯 Overview

This project implements a specialized embedded system using the **ESP32-S3 microcontroller** to analyze and measure the performance of electromagnetic coil machines. It provides:

- **Real-time oscilloscope** display of acoustic signals
- **FFT-based frequency analysis** for impact detection
- **CPS measurement** (Cycles Per Second / Impact Rate)
- **Quality scoring** based on consistency and spectral analysis
- **Live visualization** on 3.5" TFT display
- **Multi-tasking architecture** using FreeRTOS

## 🔧 Hardware Requirements

### Core Components

| Component | Specification | Purpose |
|-----------|---------------|----------|
| **Microcontroller** | ESP32-S3 DevKit | Main processor |
| **Display** | 3.5" TFT (ILI9488/ST7796) | 480x320 pixel output |
| **Microphone** | MAX9814 or INMP441 | Audio input (44.1 kHz) |
| **Optional Sensor** | MPU6050 | Vibration analysis |
| **Power Supply** | 5V/2A | System power |

### Pin Configuration

```
I2S Audio Input (Microphone):
  - SCK (Clock):    GPIO 3
  - WS (Word Select): GPIO 4  
  - SD (Data):      GPIO 2

TFT Display (SPI):
  - CS (Chip Select): GPIO 5
  - DC (Data/Cmd):   GPIO 6
  - RST (Reset):     GPIO 7
  - MOSI:            GPIO 11
  - CLK:             GPIO 12
  - MISO:            GPIO 13
  - Backlight:       GPIO 8

Optional:
  - SD Card CS:      GPIO 1
  - Touch CS:        GPIO 10
  - MPU6050 SDA:     GPIO 20
  - MPU6050 SCL:     GPIO 21
```

## 📋 Features

### Signal Processing
- ✅ **Real-time FFT** (2048-point, 46ms window @ 44.1kHz)
- ✅ **Windowing** (Hann window for spectral leakage reduction)
- ✅ **Bandpass Filtering** (100Hz - 8kHz, tunable)
- ✅ **Peak Detection** (time and frequency domain)
- ✅ **Harmonic Analysis** (up to 5th harmonic)
- ✅ **RMS Energy** calculation
- ✅ **Quality Metrics** (consistency, spectral analysis)

### Measurement Capabilities
- ✅ **CPS Range**: 0.33 - 3.33 CPS (20-200 BPM)
- ✅ **Display**: 0-100% scale
- ✅ **Impact Detection**: Automatic with configurable threshold
- ✅ **Statistics**: Mean, StdDev, Min, Max of impact intervals
- ✅ **Quality Score**: 0-100% based on consistency and spectral quality

### Display Modes
1. **Combined View** (default):
   - Waveform display (oscilloscope-style)
   - FFT spectrum analyzer
   - CPS gauge and quality bar
   - Statistics panel

2. **Waveform View**: Full-screen time-domain display
3. **Spectrum View**: Detailed frequency analysis with grid
4. **Statistics View**: Detailed metrics and history

### Serial Interface
Command-based control via serial:
- `r` - Reset impact history
- `v` - Cycle through view modes
- `s` - Print statistics summary
- `c` - Recalibrate microphone
- `+` / `-` - Adjust microphone gain
- `h` - Display help

## 🚀 Getting Started

### Prerequisites
- PlatformIO IDE or VSCode with PlatformIO extension
- ESP32-S3 board support
- Required libraries (auto-installed):
  - `bodmer/TFT_eSPI` - Display driver
  - `arm_math` - CMSIS-DSP for FFT
  - `greiman/SdFat` - SD card support (optional)

### Installation

```bash
# Clone repository
git clone https://github.com/kAInplanmusic/ESP32-S3-Coil-Machine-Analyzer.git
cd ESP32-S3-Coil-Machine-Analyzer

# Build and upload with PlatformIO
pio run -t upload

# Monitor serial output
pio device monitor
```

### Hardware Assembly

1. **Connect Microphone (I2S)**:
   - Microphone SCK → GPIO 3
   - Microphone WS → GPIO 4
   - Microphone SD → GPIO 2
   - Microphone GND → GND
   - Microphone VCC → 3.3V

2. **Connect Display (SPI)**:
   - Follow pin configuration above
   - Connect backlight to GPIO 8 (PWM capable)

3. **Power Supply**:
   - 5V supply for display and microphone
   - Use ESP32-S3 USB-C for programming

## 📊 Configuration

Edit `include/config.h` to customize:

```cpp
// Sampling
#define SAMPLE_RATE         44100   // Hz
#define FFT_SIZE            2048    // points

// Frequency Analysis
#define FREQ_MIN_BAND       100     // Hz
#define FREQ_MAX_BAND       8000    // Hz
#define FREQ_PEAK_MIN       400     // Hz
#define FREQ_PEAK_MAX       4000    // Hz

// Peak Detection
#define PEAK_THRESHOLD_DB   -30     // dB
#define PEAK_MIN_DISTANCE   2205    // samples (~50ms)

// CPS Range
#define CPS_MIN             0.33    // 20 BPM
#define CPS_MAX             3.33    // 200 BPM

// Quality Analysis
#define QUALITY_HISTORY_SIZE 100    // impacts to track
#define CONSISTENCY_THRESHOLD 15    // % std deviation
```

## 🔬 System Architecture

### Multi-Tasking Design (FreeRTOS)

```
Core 0 (Audio Processing):
  └─ audioCaptureTask (Priority 3)
     ├─ Read I2S DMA buffer
     ├─ Process samples (ring buffer)
     ├─ Compute FFT every 2048 samples
     └─ Detect peaks

Core 1 (Measurement & Display):
  ├─ measurementTask (Priority 2)
  │  ├─ Calculate CPS
  │  ├─ Compute quality score
  │  └─ Log to serial
  │
  └─ displayUpdateTask (Priority 1)
     └─ Render UI (throttled @ 100ms)
```

### Signal Processing Pipeline

```
Microphone Input (44.1 kHz, 16-bit)
        ↓
    I2S DMA Buffer
        ↓
    Ring Buffer (8192 samples)
        ↓
    ┌─────────────────┐
    │ Time Domain     │
    ├─────────────────┤
    │ • RMS Energy    │
    │ • Peak-to-Peak  │
    │ • Peak Detection│
    └─────────────────┘
        ↓
    Windowing (Hann)
        ↓
    FFT (2048-point)
        ↓
    ┌─────────────────┐
    │ Frequency Domain│
    ├─────────────────┤
    │ • Magnitude (dB)│
    │ • Peak Freq     │
    │ • Harmonics     │
    │ • Centroid      │
    └─────────────────┘
        ↓
    Quality Analysis
        ↓
    Display & Logging
```

## 📈 Measurement Interpretation

### CPS (Cycles Per Second)
- Measures impact frequency of the coil machine
- Range: 0-3.33 CPS (0-200 BPM)
- Displayed as percentage: 0-100%

### Quality Score (0-100%)
- **75-100%**: Excellent (consistent, clean impacts)
- **50-75%**: Good (mostly consistent)
- **25-50%**: Fair (variable consistency)
- **0-25%**: Poor (inconsistent or noisy)

Based on:
- **Consistency**: Standard deviation of impact intervals
- **Spectral Quality**: Peak magnitude and signal-to-noise ratio
- **Frequency Stability**: Variation in fundamental frequency

### Statistics Panel
- **Mean**: Average time between impacts (ms)
- **StdDev**: Variability in intervals (lower = more consistent)
- **Min/Max**: Range of impact intervals
- **RMS**: Root mean square energy level
- **Peak-Peak**: Maximum amplitude variation
- **Noise Floor**: Background noise level (dB)

## 🔧 Development

### Project Structure

```
ESP32-S3-Coil-Machine-Analyzer/
├── include/
│   ├── config.h              # Configuration constants
│   ├── i2s_audio.h           # Audio input class
│   ├── signal_processor.h    # DSP and FFT class
│   └── display_manager.h     # UI rendering class
├── src/
│   ├── main.cpp              # Main program & FreeRTOS tasks
│   ├── i2s_audio.cpp         # Audio implementation
│   ├── signal_processor.cpp  # Signal processing implementation
│   └── display_manager.cpp   # Display implementation
├── platformio.ini            # PlatformIO configuration
└── README.md                 # This file
```

### Building from Source

```bash
# Full build
pio run

# Upload to device
pio run -t upload

# Monitor with serial output
pio device monitor -b 115200

# Clean build
pio run -t clean
```

### Debugging

Enable verbose logging in `src/main.cpp`:
```cpp
#define LOG_SERIAL  true  // Serial debug output
#define LOG_SD_CARD true  // SD card logging (future)
```

## 🔌 Power Consumption

| Component | Current | Notes |
|-----------|---------|-------|
| ESP32-S3 | 80-160 mA | Active processing |
| I2S Microphone | 5-10 mA | Continuous sampling |
| TFT Display | 100-200 mA | Backlight-dependent |
| **Total** | **200-400 mA** | @ 5V |

## 📝 Future Enhancements

- [ ] SD Card logging for long-term data storage
- [ ] WiFi streaming and remote monitoring
- [ ] Touch screen calibration and menu system
- [ ] MPU6050 vibration correlation analysis
- [ ] Advanced filtering (Kalman, adaptive)
- [ ] Machine learning-based anomaly detection
- [ ] USB-C data export (CSV)
- [ ] Reference pattern storage and comparison
- [ ] Harmonic distortion analysis
- [ ] Phase tracking and synchronization

## 🐛 Troubleshooting

### No Audio Input
- Check I2S pin connections
- Verify microphone power supply
- Run calibration command: `c`

### Display Flickering
- Increase backlight brightness
- Check SPI connection quality
- Reduce display update frequency in config

### Low Quality Score
- Check for ambient noise
- Verify microphone is properly positioned
- Calibrate microphone gain
- Check mechanical stability of machine

### Serial Connection Issues
- Verify USB cable is data-capable
- Check baud rate (115200)
- Install CH340/FTDI drivers if needed

## 📄 License

MIT License - See LICENSE file

## 👤 Author

kAInplanmusic - ESP32 Systems Engineer

## 🤝 Contributing

Contributions welcome! Please submit issues and pull requests.

---

**Last Updated**: September 2026
**Firmware Version**: 1.0.0
**ESP32-S3 Support**: Full IDF 5.x

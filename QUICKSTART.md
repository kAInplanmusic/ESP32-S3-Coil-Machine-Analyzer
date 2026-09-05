# ESP32-S3 Coil Machine Analyzer - Quick Start Guide

## Initial Setup

### 1. Hardware Assembly (15 minutes)

**Display Connection:**
```
GPIO 5  → TFT CS
GPIO 6  → TFT DC
GPIO 7  → TFT RST
GPIO 11 → TFT MOSI
GPIO 12 → TFT CLK
GPIO 13 → TFT MISO
GPIO 8  → TFT Backlight (PWM)
3.3V    → TFT VCC
GND     → TFT GND
```

**Microphone Connection:**
```
GPIO 3  → Microphone SCK (Clock)
GPIO 4  → Microphone WS (Word Select)
GPIO 2  → Microphone SD (Data)
3.3V    → Microphone VCC
GND     → Microphone GND
```

### 2. Software Installation

```bash
# Install PlatformIO CLI
pip install platformio

# Clone and enter directory
git clone https://github.com/kAInplanmusic/ESP32-S3-Coil-Machine-Analyzer.git
cd ESP32-S3-Coil-Machine-Analyzer

# Build project
pio run

# Connect ESP32-S3 via USB
# Upload firmware
pio run -t upload

# Monitor output
pio device monitor
```

### 3. First Run

1. **Display Initialization**
   - You should see "Initializing..." on the screen
   - Backlight turns on
   - Status: "Ready"

2. **Audio Calibration**
   - System automatically calibrates microphone
   - Watch serial output for "Calibration complete"

3. **Start Measurement**
   - System begins capturing audio
   - Display shows waveform and spectrum
   - CPS value updates every 500ms

## Basic Operation

### Serial Commands

Open serial monitor at **115200 baud**:

```
Command | Action
--------|--------
  r     | Reset impact history
  v     | Change display view (4 modes)
  s     | Print statistics summary
  c     | Recalibrate microphone
  +     | Increase gain +5dB
  -     | Decrease gain -5dB
  h     | Show help
```

### Typical Workflow

1. **Calibration Phase**
   - Place microphone near coil machine
   - Run `c` command to calibrate
   - Adjust gain with `+`/`-` until audio level is optimal

2. **Measurement Phase**
   - Machine starts operating
   - Watch CPS value and quality score
   - Press `v` to switch display modes

3. **Analysis Phase**
   - Press `s` to see detailed statistics
   - Analyze consistency (StdDev)
   - Check frequency spectrum
   - Export data if needed

## Understanding the Display

### Combined View (Default)

```
┌─────────────────────────────────┐
│ Coil Analyzer           Running  │  ← Status Bar
├─────────────────────────────────┤
│  Waveform (Oscilloscope View)   │
│  ╔═══════════════════════════╗  │
│  ║ ~~ ╱╲ ╱╲ ╱╲ ~~           ║  │  ← Audio signal
│  ╚═══════════════════════════╝  │
├─────────────────────────────────┤
│  Spectrum (FFT Display)         │
│  ┃┃ ┃  ┃  ┃     ┃ ┃           │  ← Frequency content
│  100  500 1K  2K  4K Hz         │
├─────────────────────────────────┤
│ CPS: 1.45 (42%)  Quality: 78%  │  ← Main metrics
│ ████████████░░░░░░░░░░░░░░░░  │  ← Quality bar
│                                 │
│ Mean: 645.2ms   RMS: 0.234     │  ← Statistics
│ StdDev: 32.1ms  Peak-Peak: 150 │
│ Min: 592ms      Noise: -45dB   │
│ Max: 701ms                      │
└─────────────────────────────────┘
```

### Waveform View
- Shows 186ms of audio data
- Grid overlay for reference
- Green trace = audio samples
- Useful for examining individual impacts

### Spectrum View
- FFT magnitude in dB
- Frequency axis: 100Hz to 8kHz
- Yellow bars = spectral content
- Peak markers for harmonics

## Troubleshooting

### Issue: "Display failed!" error

**Solution:**
- Check TFT pins match config.h
- Verify SPI bus is not shared
- Try lowering SPI speed in User_Setup.h
- Check display cable connections

### Issue: No audio input

**Solution:**
- Verify I2S pins (GPIO 2, 3, 4)
- Check microphone power (should be on VCC)
- Test microphone with oscilloscope
- Run `c` calibration command

### Issue: Low CPS readings

**Solution:**
- Increase microphone gain: `+` key
- Move microphone closer to source
- Check PEAK_THRESHOLD_DB in config.h
- Verify FREQ_PEAK_MIN/MAX match actual frequencies

### Issue: Noisy spectrum

**Solution:**
- Check for electromagnetic interference
- Use shielded microphone cable
- Ensure proper grounding
- Increase PEAK_THRESHOLD_DB

## Performance Metrics

### CPU Usage (Estimated)
- **Core 0** (Audio): ~30-40% @ 240MHz
- **Core 1** (Processing + Display): ~20-30% @ 240MHz
- **Total System**: ~50-70% utilization

### Memory Usage
- **RAM**: ~150 KB (buffers + heap)
- **PSRAM**: Used for FFT buffers if available
- **Free**: ~100-150 KB available for expansion

### Latency
- **Audio Capture**: <1ms (I2S DMA)
- **FFT Computation**: ~5-10ms per frame
- **Display Update**: ~50-100ms (throttled)
- **Total E2E**: ~100-150ms

## Advanced Configuration

### Adjust FFT Window Size

In `config.h`:
```cpp
#define FFT_SIZE 4096  // Larger = better freq resolution
                       // But slower computation
```

### Change Frequency Range

```cpp
#define FREQ_MIN_BAND   50    // Lower limit (Hz)
#define FREQ_MAX_BAND   12000 // Upper limit (Hz)
#define FREQ_PEAK_MIN   300   // Expected impact min freq
#define FREQ_PEAK_MAX   5000  // Expected impact max freq
```

### Adjust Peak Detection Sensitivity

```cpp
#define PEAK_THRESHOLD_DB -20  // Lower = more sensitive
#define PEAK_MIN_DISTANCE 1000 // Minimum samples between peaks
```

## Data Logging (Future)

When SD card support is added:

```cpp
#define LOG_SD_CARD true
#define CSV_LOG_PATH "/logs/"
#define CSV_LOG_INTERVAL 1000  // ms
```

Data will be saved to `logs/YYYYMMDD_HHMMSS.csv`

## Support & Resources

- **Issue Tracker**: GitHub Issues
- **Documentation**: README.md
- **Examples**: `examples/` directory
- **Community**: GitHub Discussions

## Next Steps

1. ✅ Hardware assembly complete
2. ✅ Firmware uploaded and running
3. ⬜ Calibrate for your specific machine
4. ⬜ Collect baseline measurements
5. ⬜ Implement custom analysis
6. ⬜ Set up data logging (optional)

---

**Questions?** Check GitHub Issues or create a new one!

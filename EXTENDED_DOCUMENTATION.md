# ESP32-S3 Coil Machine Analyzer v3.0 - Extended Documentation

## Complete Feature Guide & API Reference

---

## Table of Contents

1. [Advanced Calibration System](#advanced-calibration-system)
2. [SD Card Data Logging](#sd-card-data-logging)
3. [Bluetooth LE Interface](#bluetooth-le-interface)
4. [Build System (PlatformIO)](#build-system-platformio)
5. [API Reference](#api-reference)
6. [Examples & Usage](#examples--usage)
7. [Troubleshooting](#troubleshooting-extended)

---

## Advanced Calibration System

### CalibrationManager Overview

The **CalibrationManager** provides intelligent, adaptive microphone calibration with noise profiling and gain optimization.

### Features

#### 1. **Automatic Noise Floor Detection**
```cpp
CalibrationManager calibration;
calibration.init(&audio);
calibration.startCalibration(5000);  // 5-second calibration

// During first phase (1.67 seconds):
// - Samples silence/ambient noise
// - Calculates noise floor (dB)
// - Measures noise standard deviation
```

**Noise Floor Calculation:**
```
Noise_Floor_dB = 20 * log10(RMS_Noise + 1e-6)
```

#### 2. **Reference Waveform Capture**
```cpp
// During second phase (1.67 seconds):
// - Listens for test strike/signal
// - Records peak amplitude
// - Stores reference waveform
```

#### 3. **Optimal Gain Calculation**
```cpp
// During analysis phase:
Optimal_Gain = Target_dB - Current_Peak_dB
// Target: -6 dB (0.5 linear amplitude)
// Ensures full range utilization without clipping
```

#### 4. **Persistent Storage (EEPROM)**

```cpp
// Save calibration
calibration.saveToEEPROM();

// Load on startup
if (calibration.loadFromEEPROM()) {
    calibration.applyCalibration();
}
```

### Calibration Data Structure

```cpp
struct CalibrationData {
    float noise_floor_db;          // Minimum detectable signal
    float noise_std_dev;           // Noise variability (σ)
    float optimal_gain_db;         // Recommended gain adjustment
    float reference_peak_db;       // Peak from test signal
    float frequency_response[32];  // 32-bin freq response (0-2kHz)
    uint32_t calibration_timestamp;// When calibration was done
    bool is_valid;                 // Validity flag
};
```

### Calibration States

```
IDLE
  └─> startCalibration()
      └─> MEASURING_NOISE (1.67s)
          └─> CAPTURING_REFERENCE (1.67s)
              └─> ANALYZING
                  └─> COMPLETE (valid=true)
                      └─> applyCalibration() / saveToEEPROM()
```

### Usage Example

```cpp
// In setup()
CalibrationManager calibration;
calibration.init(&audio);

// Try to load saved calibration
if (!calibration.loadFromEEPROM()) {
    Serial.println("No saved calibration, starting new calibration...");
    calibration.startCalibration(5000);
}

// In loop()
if (calibration.update()) {
    // Calibration complete
    calibration.applyCalibration();
    calibration.saveToEEPROM();
    Serial.println(calibration.getInfoString());
    // Output: "Noise: -45.2 dB | Gain: 12.5 dB | Peak: -5.8 dB"
}
```

### Serial Commands for Calibration

```
Command   Description                      Expected Output
-------   -----------                      ----------------
c         Start NEW calibration            "Calibration complete!"
+         Increase gain by 5dB             "Gain increased to 15.2 dB"
-         Decrease gain by 5dB             "Gain decreased to 5.2 dB"
```

---

## SD Card Data Logging

### SDLogger Overview

The **SDLogger** provides comprehensive data export in multiple formats (CSV, JSON, Binary).

### Supported Formats

#### 1. **CSV Format** (Human-Readable, Default)

**Filename:** `measure_20260905_180030.csv`

**Structure:**
```csv
ESP32-S3 Coil Machine Analyzer - Measurement Log
Timestamp, 1234567890

Measurement 1, Time 1234 ms, Points 8
#, Time_ms, Amplitude, dB, Slope_deg, Energy, Freq_Hz
1, 0.50, 0.4500, -6.94, 45.2, 0.0120, 1250.0
2, 1.25, 0.6200, -4.16, 62.1, 0.0185, 1500.0
3, 2.10, 0.5100, -5.84, -45.3, 0.0142, 1400.0
...
Summary, 0.85, 0.6200, -4.16, 1, 0.3200, 0.0025

=== STATISTICS SUMMARY ===
CPS, 85.42
Quality Score, 92%
Consistency, 94.2%
Samples Collected, 100
Mean Period, 11.70 ms
Period Jitter, 0.15 ms

Mean Harmonic 2/1, 0.3450
Mean Harmonic 3/1, 0.1200
Mean Decay Rate, 0.0035
Mean Q-Factor, 8.95
```

#### 2. **JSON Format** (Machine-Readable, API-Compatible)

**Filename:** `measure_20260905_180030.json`

```json
{
  "timestamp": 1234567890,
  "device": "ESP32-S3 Coil Analyzer",
  "version": "3.0",
  "measurements": [
    {
      "measurement": 1,
      "time_ms": 1234,
      "point_count": 8,
      "points": [
        {"number": 1, "time_ms": 0.50, "amplitude": 0.4500, "db": -6.94},
        {"number": 2, "time_ms": 1.25, "amplitude": 0.6200, "db": -4.16}
      ]
    }
  ]
}
```

### Usage Example

```cpp
// In setup()
SDLogger logger;
if (!logger.init()) {
    Serial.println("SD card initialization failed!");
}

// After measurement complete
if (advancedAnalyzer.getStats().measurement_complete) {
    // Export as CSV
    String filename = logger.logMeasurement(advancedAnalyzer, SDLogger::FORMAT_CSV);
    Serial.printf("Logged to: %s\n", filename.c_str());
    
    // Or export as JSON
    filename = logger.logMeasurement(advancedAnalyzer, SDLogger::FORMAT_JSON);
    
    // Check remaining storage
    float free_mb = logger.getFreeSpace() / 1024.0 / 1024.0;
    Serial.printf("Free space: %.1f MB\n", free_mb);
}

// Cleanup old logs (keep only 10 most recent)
logger.cleanupOldLogs(10);

// List all logs
std::vector<String> logs = logger.listLogs();
for (const auto& log : logs) {
    Serial.println(log);
}
```

### CSV Column Reference

| Column | Unit | Range | Description |
|--------|------|-------|-------------|
| # | - | 1-20 | Measurement point number |
| Time_ms | ms | 0-100 | Time from impulse start |
| Amplitude | linear | 0-1 | Normalized amplitude |
| dB | dB | -80-0 | Logarithmic amplitude |
| Slope_deg | degrees | -180-180 | Rise/fall steepness |
| Energy | - | 0-1 | Segment energy (area under curve) |
| Freq_Hz | Hz | 0-22000 | Estimated local frequency |

### Data Recovery & Analysis

**Python script for analysis:**

```python
import pandas as pd

# Load CSV
df = pd.read_csv('measure_20260905_180030.csv', skiprows=2)

# Extract statistics
stats = df[df['#'].astype(str).str.startswith('Summary')]
print(f"CPS: {stats['Time_ms'].values[0]}")
print(f"Peak Amplitude: {stats['Amplitude'].values[0]}")

# Plot waveform
import matplotlib.pyplot as plt
plt.plot(df['Time_ms'], df['Amplitude'])
plt.xlabel('Time (ms)')
plt.ylabel('Amplitude (linear)')
plt.title('Impact Waveform')
plt.show()
```

### SD Card Requirements

- **Minimum Size:** 1GB (SD or microSD)
- **Class:** 10 or higher (UHS recommended)
- **Format:** FAT32 or exFAT
- **Pin:** CS on GPIO 10 (configurable in code)

---

## Bluetooth LE Interface

### BluetoothManager Overview

The **BluetoothManager** provides wireless remote monitoring and control via BLE (Bluetooth Low Energy).

### Service & Characteristics

```
BLE Service: Device Information (180A)
├─ CPS Characteristic (2A58)
│  └─ Properties: Read, Notify
│  └─ Size: 4 bytes (float)
│  └─ Update Rate: On-demand or real-time
│
├─ Quality Characteristic (2A19)
│  └─ Properties: Read, Notify
│  └─ Size: 1 byte (0-100%)
│  └─ Update Rate: ~1 Hz
│
├─ Measurement Point (2A37)
│  └─ Properties: Read, Notify
│  └─ Size: 20 bytes (packed point data)
│  └─ Update Rate: Per point detection
│
└─ Command Characteristic (2A39)
   └─ Properties: Write
   └─ Size: 4 bytes (command + parameters)
   └─ Commands: 0x01-0x04, 0x10-0x1F
```

### Characteristic Data Formats

#### CPS Characteristic (4 bytes)
```
Byte 0-3: Float (IEEE 754 single precision)
Example: 85.42 CPS = 0x42AAAE33
```

#### Quality Characteristic (1 byte)
```
Byte 0: uint8_t (0-100%)
Example: 92% = 0x5C
```

#### Measurement Point (20 bytes)
```
Offset  Size  Type    Field             Range
------  ----  ----    -----             -----
0       1     uint8   Point Number      1-20
1-2     2     uint16  Time (×0.1ms)     0-10000ms
3-4     2     uint16  Amplitude (×0.001) 0-1.0
5-6     2     int16   dB (×0.1dB)       -800 to 0
7-8     2     int16   Slope (×0.1°)     -1800 to 1800
9-10    2     uint16  Frequency (Hz)    0-22000
11-12   2     uint16  Energy (×0.01)    0-1.0
13-19   7     --      Reserved          0x00
```

#### Command Characteristic (Write-Only)
```
Byte 0: Command Type
  0x01 = Start measurement
  0x02 = Stop measurement
  0x03 = Reset analyzer
  0x04 = Start calibration
  0x10-0x1F = Set gain (0x10 + value, e.g., 0x15 = +5dB)

Byte 1-3: Parameters (command-specific)
```

### Usage Example (Arduino)

```cpp
// In setup()
BluetoothManager ble;
ble.init("CoilAnalyzer-01");

// In loop()
ble.update();

if (ble.isConnected()) {
    // Send real-time CPS
    ble.sendRealtimeCPS(85.42f, 92);
    
    // Send measurement point
    const auto& point = advancedAnalyzer.getLastWaveform().points[0];
    ble.sendMeasurementPoint(point, 1);
    
    // Send statistics
    ble.sendStatistics(advancedAnalyzer.getStats());
}
```

### Mobile App Integration (iOS/Android)

**Required BLE Permissions:**

**iOS (Info.plist):**
```xml
<key>NSBluetoothPeripheralUsageDescription</key>
<string>Coil Analyzer needs Bluetooth to connect</string>
```

**Android (AndroidManifest.xml):**
```xml
<uses-permission android:name="android.permission.BLUETOOTH" />
<uses-permission android:name="android.permission.BLUETOOTH_ADMIN" />
<uses-permission android:name="android.permission.ACCESS_FINE_LOCATION" />
```

**Example Swift Code:**

```swift
import CoreBluetooth

class CoilAnalyzerBLE: NSObject, CBCentralManagerDelegate {
    var centralManager: CBCentralManager!
    let serviceUUID = CBUUID(string: "180A")
    let cpsCharacteristic = CBUUID(string: "2A58")
    
    func startScanning() {
        centralManager.scanForPeripherals(
            withServices: [serviceUUID],
            options: nil
        )
    }
    
    func readCPS() {
        // Read CPS value from characteristic
        // Parse as IEEE 754 float
    }
}
```

### Bluetooth Range & Performance

| Factor | Value |
|--------|-------|
| **Range** | 10-100m (line of sight) |
| **Latency** | <20ms per notification |
| **Power** | ~10mA (TX) |
| **Data Rate** | 1 Mbps (BLE 4.2+) |
| **Max Notifications/s** | ~10 (limited by peripheral) |

---

## Build System (PlatformIO)

### Installation

```bash
# Install PlatformIO CLI
pip install platformio

# Or install PlatformIO IDE
# VS Code Extension: platformio.platformio-ide
```

### Building

```bash
# Default build (debug)
pio run

# Release build (optimized)
pio run -e release

# Specific target
pio run -e esp32-s3-devkitc-1
```

### Upload

```bash
# Auto-detect port and upload
pio run -t upload

# Upload with custom port
pio run -t upload --upload-port /dev/ttyUSB0

# Monitor after upload
pio run -t uploadfs && pio device monitor -b 115200
```

### Configuration Options

**platformio.ini sections:**

```ini
[env:esp32-s3-devkitc-1]
platform = espressif32            # Platform
board = esp32-s3-devkitc-1        # Board variant
framework = arduino               # Framework
monitor_speed = 115200            # Serial baud
upload_speed = 921600             # Upload baud (fast)

build_flags =
    -O2                           # Optimization level
    -Wall -Wextra                 # Compiler warnings
    -DCORE_DEBUG_LEVEL=ARDUHAL_LOG_LEVEL_INFO  # Debug level

lib_deps =
    ArduinoBLE                    # Bluetooth support
    SdFat                         # SD card library
    https://github.com/morrissimo/ESP32-I2S-Slavemode.git
```

### Memory Configuration

**ESP32-S3 Partitioning:**

```csv
# partitions.csv
Name,   Type, SubType, Offset,  Size,    Flags
bootloader_qio_40m, 0x0, 0x0, 0x0, 0x4000,
partitions,         0x1, 0x0, 0x4000, 0x800,
otadata,           0x1, 0x2, 0x4800, 0x2000,
app0,               0x0, 0x10, 0x8000, 0x400000,
spiffs,             0x1, 0x82, 0x408000, 0x1F8000,
```

### Debugging

```bash
# Enable debug output
pio run -e debug

# Monitor with timestamps
pio device monitor -b 115200 --filters time log2file

# Log to file
pio device monitor -b 115200 --filters log2file > output.log
```

---

## API Reference

### CalibrationManager API

```cpp
// Initialize
bool init(I2SAudio* audio);

// Start calibration (duration in ms)
void startCalibration(uint16_t duration_ms = 5000);

// Update state machine
bool update();  // Returns true when complete

// Get current state
CalibrationState getState() const;

// Get progress (0-100%)
uint8_t getProgress() const;

// Get calibration data
const CalibrationData& getData() const;

// Apply to audio input
void applyCalibration();

// Persistence
bool saveToEEPROM();
bool loadFromEEPROM();

// Info string
String getInfoString() const;
```

### SDLogger API

```cpp
// Initialize SD card
bool init();

// Check availability
bool isAvailable() const;

// Log measurement (returns filename)
String logMeasurement(const AdvancedImpactAnalyzer& analyzer,
                     LogFormat format = FORMAT_CSV);

// List log files
std::vector<String> listLogs();

// Cleanup (keep N most recent)
void cleanupOldLogs(uint8_t keep_count = 10);

// Get free space
uint64_t getFreeSpace() const;
```

### BluetoothManager API

```cpp
// Initialize with device name
bool init(const char* device_name = "CoilAnalyzer");

// Control advertising
void startAdvertising();
void stopAdvertising();

// Update (handle connections)
void update();

// Check connection state
bool isConnected() const;

// Send data
void sendMeasurementData(const AdvancedImpactAnalyzer& analyzer);
void sendRealtimeCPS(float cps, uint8_t quality);
void sendMeasurementPoint(const MeasurementPoint& point, uint8_t number);
void sendStatistics(const AdvancedImpactAnalyzer::AdvancedStats& stats);
```

---

## Examples & Usage

### Complete Integration Example

```cpp
#include "calibration_manager.h"
#include "sd_logger.h"
#include "bluetooth_manager.h"

CalibrationManager calibration;
SDLogger logger;
BluetoothManager ble;

void setup() {
    // Initialize all systems
    calibration.init(&audio);
    logger.init();
    ble.init("CoilAnalyzer");
    
    // Load saved calibration or start new
    if (!calibration.loadFromEEPROM()) {
        Serial.println("Starting calibration...");
        calibration.startCalibration(5000);
    }
}

void loop() {
    // Update calibration if running
    if (calibration.update()) {
        calibration.applyCalibration();
        calibration.saveToEEPROM();
    }
    
    // Update Bluetooth
    ble.update();
    if (ble.isConnected()) {
        ble.sendRealtimeCPS(currentCPS, qualityScore);
    }
    
    // Handle measurement completion
    if (advancedAnalyzer.getStats().measurement_complete) {
        // Export data
        String filename = logger.logMeasurement(advancedAnalyzer,
                                               SDLogger::FORMAT_CSV);
        Serial.printf("Saved to: %s\n", filename.c_str());
        
        // Send via Bluetooth
        ble.sendMeasurementData(advancedAnalyzer);
    }
}
```

### Serial Command Handler

```cpp
void handleSerialCommands() {
    if (!Serial.available()) return;
    
    char cmd = Serial.read();
    
    switch (cmd) {
        case 'C':  // Start calibration
            calibration.startCalibration(5000);
            break;
            
        case 'L':  // List logs
            {
                auto logs = logger.listLogs();
                for (const auto& log : logs) {
                    Serial.println(log);
                }
            }
            break;
            
        case 'B':  // Toggle Bluetooth
            if (ble.isConnected()) {
                ble.stopAdvertising();
            } else {
                ble.startAdvertising();
            }
            break;
    }
}
```

---

## Troubleshooting (Extended)

### Calibration Issues

**Problem:** Calibration shows very high/low gain
```
Solution:
1. Check microphone connections
2. Verify power supply (3.3V, stable)
3. Reduce ambient noise
4. Try multiple calibration runs
```

**Problem:** EEPROM save fails
```
Solution:
1. Check EEPROM not full (256+ bytes free)
2. Verify ESP32-S3 has PSRAM (required for this build)
3. Try eraseEEPROM() first
```

### SD Card Issues

**Problem:** "SD card initialization failed"
```
Solution:
1. Check CS pin (GPIO 10) connection
2. Verify SPI bus not shared with display
3. Try different card (Class 10 recommended)
4. Check card is formatted FAT32
```

**Problem:** Slow data logging
```
Solution:
1. Use high-speed SD card (Class 10+)
2. Increase SPI clock (up to 40 MHz)
3. Log during idle time, not during measurement
```

### Bluetooth Issues

**Problem:** Device not advertising
```
Solution:
1. Check BLE antenna connection
2. Verify BLE stack enabled (check config)
3. Restart device
4. Check for other wireless interference
```

**Problem:** Frequent disconnections
```
Solution:
1. Increase update() call frequency
2. Check power supply stability
3. Reduce WiFi interference
4. Move device closer to mobile device
```

### Build Issues

**Problem:** "Cannot find Arduino.h"
```
Solution:
1. Run: pio lib install
2. Check .vscode/c_cpp_properties.json
3. Verify board selection: esp32-s3-devkitc-1
```

**Problem:** Memory overflow
```
Solution:
1. Reduce FFT_SIZE in config.h
2. Limit max measurement points
3. Disable one of: SD, BLE, or analysis
4. Use release build: pio run -e release
```

---

## Performance Optimization

### Memory Usage

| Component | RAM | Flash |
|-----------|-----|-------|
| Core System | 50KB | 200KB |
| Audio Buffer | 40KB | - |
| Advanced Analyzer | 100KB | 50KB |
| Display Buffers | 60KB | - |
| SD Logger | 10KB | 30KB |
| Bluetooth | 20KB | 40KB |
| **Total** | **280KB** | **320KB** |

### Optimization Tips

```cpp
// Use constexpr for compile-time constants
const constexpr uint16_t FFT_SIZE = 2048;

// Preallocate vectors to avoid fragmentation
_waveforms.reserve(100);

// Use references instead of copies
const auto& stats = analyzer.getStats();

// Disable logging during heavy computation
#ifdef DEBUG_BUILD
  ESP_LOGD(TAG, "...");
#endif
```

---

## Version History

| Version | Date | Changes |
|---------|------|----------|
| 3.0 | 2026-09-05 | Full system with Cal, SD, BLE |
| 2.5 | 2026-08-20 | Advanced UI and waveform analysis |
| 2.0 | 2026-08-01 | FFT and basic CPS measurement |
| 1.0 | 2026-07-01 | Initial release |

---

## Support & Contributing

**Issues:** Report on GitHub with:
- Compiler output
- Serial monitor logs
- Hardware configuration
- Steps to reproduce

**Contributing:** Pull requests welcome for:
- Performance improvements
- Additional export formats
- Mobile app examples
- Documentation

---

**Last Updated:** 2026-09-05  
**Maintainer:** MONKmonastry  
**License:** MIT  

# ESP32-S3 Coil Machine Analyzer - Seeed XIAO ESP32-S3 Edition

## Hardware Setup für Seeed XIAO ESP32-S3

### Board Specifications

```
Seeed XIAO ESP32-S3
==================
- Prozessor: Xtensa Dual-Core 32-bit @ 240 MHz
- RAM: 8 MB PSRAM + 384 KB SRAM
- Flash: 16 MB
- Größe: 21 x 17.5 mm (sehr compact!)
- USB-C: CDC Serial + Power
- GPIO: 11 digitale Pins
- ADC: 8-channel 12-bit
- I2S: Full I2S support
- Preis: ~$15
```

### Pin-Belegung (XIAO Connector)

```
┌─────────────────────────────────┐
│  Seeed XIAO ESP32-S3            │
├─────────────────────────────────┤
│ D0 (GPIO0)   - I2C SCL          │
│ D1 (GPIO1)   - I2C SDA          │
│ D2 (GPIO2)   - I2S DOUT         │
│ D3 (GPIO3)   - TFT BL (PWM)     │
│ D4 (GPIO4)   - TFT RST          │
│ D5 (GPIO5)   - TFT DC           │
│ D6 (GPIO6)   - TFT CS           │
│ D7 (GPIO7)   - SPI SCLK         │
│ D8 (GPIO8)   - SPI MOSI         │
│ D9 (GPIO9)   - SPI MISO         │
│ D10 (GPIO10) - SD CS (external) │
├─────────────────────────────────┤
│ GPIO40       - I2S DIN (Audio)  │
│ GPIO41       - I2S WS (LR Clk)  │
│ GPIO42       - I2S BCLK         │
│ GPIO19/20    - USB (auto)       │
└─────────────────────────────────┘
```

### Verdrahtungsschema

#### Display (3.5" TFT, ILI9488 oder ST7796)

```
Display Pin    XIAO Pin    GPIO    Color
─────────────────────────────────────────
VCC            5V          -       Red
GND            GND         -       Black
CS             D6          6       Yellow
DC             D5          5       Green
RST            D4          4       Blue
SDI (MOSI)     D8          8       Purple
SCK (SCLK)     D7          7       Orange
LED (BL)       D3          3       White
SDO (MISO)     D9          9       Gray
GND            GND         -       Black
```

**Compact Setup:**
```
Display Module (SPI):
┌────────────────────┐
│ VCC ─ 5V (USB)     │
│ GND ─ GND          │
│ CS  ─ D6 (GPIO6)   │
│ DC  ─ D5 (GPIO5)   │
│ RES ─ D4 (GPIO4)   │
│ DI  ─ D8 (MOSI)    │
│ CK  ─ D7 (SCLK)    │
│ DO  ─ D9 (MISO)    │
│ LED ─ D3 (PWM)     │
└────────────────────┘
```

#### Microphone (I2S Audio Input)

```
Microphone Pin    XIAO Pin    GPIO    Function
──────────────────────────────────────────────
VCC               5V (USB)    -       Power
GND               GND         -       Ground
SCK (BCLK)        GPIO42      42      Bit Clock
WS (LR)           GPIO41      41      Word Select
SDI (DIN)         GPIO40      40      Data In
SDO (DOUT)        D2 (opt)    2       Data Out
```

**MAX9814 Microphone Amplifier:**
```
MAX9814 Pin    XIAO Pin    Function
─────────────────────────────────────
VCC            5V          Power
GND            GND         Ground
OUT            GPIO40      I2S DIN
GAIN           GND/5V      Gain selection
AR/AL          5V          Auto-gain control
```

**INMP441 Digital Microphone:**
```
INMP441 Pin    XIAO Pin    Function
───────────────────────────────────
VCC            3V3         Power
GND            GND         Ground
CLK            GPIO42      I2S BCLK
WS             GPIO41      I2S WS
SD             GPIO40      I2S DIN
```

#### SD Card Module (SPI - Shared mit Display)

```
SD Module Pin    XIAO Pin    GPIO    Function
─────────────────────────────────────────────
VCC              5V          -       Power
GND              GND         -       Ground
CS               D10         10      Chip Select
MOSI             D8          8       SPI Data (shared)
SCK              D7          7       SPI Clock (shared)
MISO             D9          9       SPI Response (shared)
```

**Note:** SD Card und Display teilen sich die gleiche SPI-Bus (D7, D8, D9).
Jedes hat einen eigenen CS-Pin (D6 für Display, D10 für SD).

#### Optional: MPU6050 Vibrationssensor

```
MPU6050 Pin    XIAO Pin    GPIO    Function
──────────────────────────────────────────
VCC            3V3         -       Power
GND            GND         -       Ground
SCL            D0          0       I2C Clock
SDA            D1          1       I2C Data
INT            D2          2       Interrupt (optional)
AD0            GND or 3V3  -       I2C Address select
```

### Stromversorgung

```
┌─────────────────────────────────────────┐
│  USB-C Power Management (XIAO)          │
├─────────────────────────────────────────┤
│  Input: 5V USB                          │
│  ├─ Regulator: AMS1117 (3.3V)          │
│  └─ Output: 3.3V/600mA typ             │
│                                         │
│  Power Budget:                          │
│  - ESP32-S3 Core: 80-150mA             │
│  - Display: 200-400mA                  │
│  - Microphone: 5-10mA                  │
│  - Audio Processing: 50-100mA         │
│  - Bluetooth: 20-50mA                  │
│  ─────────────────────────             │
│  TOTAL: 400-800mA @ 5V USB             │
│                                         │
│  ⚠ Hinweis: High-Power Displays       │
│    benötigen externe 5V/2A PSU!        │
└─────────────────────────────────────────┘
```

**Empfohlen:**
- USB-Kabel: High-quality, short (<1m)
- Power Bank: Min. 10000mAh, 2A output
- Lab PSU: 5V/2A für Dauerbetrieb

### Kompakte Steckplatinen-Anordnung

```
┌────────────────────────────────┐
│  XIAO ESP32-S3 (oben)          │
│  [USB-C] [GPIO 21] [GPIO 11]   │
│                                 │
│  Breadboard Layout:            │
│  ┌──────────────────────────┐  │
│  │ D0 ├─ I2C SCL            │  │
│  │ D1 ├─ I2C SDA            │  │
│  │ D2 ├─ I2S DOUT (unused)  │  │
│  │ D3 ├─ TFT BL             │  │
│  │ D4 ├─ TFT RST            │  │
│  │ D5 ├─ TFT DC             │  │
│  │ D6 ├─ TFT CS             │  │
│  │ D7 ├─ SCLK (SPI)         │  │
│  │ D8 ├─ MOSI (SPI)         │  │
│  │ D9 ├─ MISO (SPI)         │  │
│  │ D10├─ SD CS              │  │
│  │ 5V ├─ Power              │  │
│  │ GND├─ Ground             │  │
│  └──────────────────────────┘  │
│                                 │
│  GPIO40/41/42 (side):          │
│  ├─ I2S Audio Input            │
│  └─ Hidden pins               │
└────────────────────────────────┘
```

### Build & Flash

```bash
# 1. Install PlatformIO
pip install platformio

# 2. Clone repository
git clone https://github.com/kAInplanmusic/ESP32-S3-Coil-Machine-Analyzer.git
cd ESP32-S3-Coil-Machine-Analyzer

# 3. Build for XIAO
pio run -e seeed-xiao-esp32s3

# 4. Flash (XIAO goes into bootloader automatically)
pio run -t upload -e seeed-xiao-esp32s3

# 5. Monitor serial output
pio device monitor -b 115200
```

### Serial Commands (USB CDC)

```
Command        Function                          Output
────────────────────────────────────────────────────────
s              START measurement (50 points)     "Measurement started..."
S              STOP measurement                  "Measurement stopped"
c              Start CALIBRATION                 "Calibration complete!"
i              Show INFO & statistics            CPS, Quality, Consistency
+              Increase gain by 5dB              "Gain: +5.0 dB"
-              Decrease gain by 5dB              "Gain: -5.0 dB"
?              Show HELP                         Command list
r              RESET analyzer                    "Reset complete"
m              MEMORY info                       Heap usage
h              HARMONICS analysis                H1-H5 display
n              NOISE profile                     Noise floor (dB)
```

### Typical Output

```
ESP32-S3 Coil Machine Analyzer v3.0
====================================
Board: Seeed XIAO ESP32-S3
Display: ILI9488 @ 480x320
Audio: I2S 44.1kHz, 16-bit

Calibration Data:
  Noise Floor: -45.2 dB
  Optimal Gain: +8.5 dB
  Ref Peak: -5.8 dB

Measurement Complete (50 samples):
  CPS: 85.42 (1282 CPM)
  Quality Score: 92%
  Consistency: 94.2%
  Period Jitter: 0.15 ms

Harmonics:
  H2/H1 Ratio: 0.345
  H3/H1 Ratio: 0.120
  THD: 8.5%

Decay Analysis:
  Decay Rate: 0.0035 (per ms)
  Q-Factor: 8.95
  Einschwingzeit: 285 ms

CSV Export: measure_20260905_180030.csv
Bluetooth: Connected (85.42 CPS)
```

### Speicherbelegung (XIAO Specifics)

```
ESP32-S3 Internal Memory:
┌─────────────────────────────┐
│ Total SRAM: 384 KB          │
├─────────────────────────────┤
│ Core System:      50 KB     │
│ Audio Buffers:    40 KB     │
│ FFT/Analysis:    100 KB     │
│ Display Buffers:  60 KB     │
│ BLE Stack:        20 KB     │
│ Free Heap:       114 KB     │
│                  ───────    │
│ TOTAL:           384 KB     │
└─────────────────────────────┘

Flash Memory (16 MB):
├─ Bootloader:     16 KB
├─ Partition Table: 4 KB
├─ OTA App Space: 6.5 MB
├─ Application:   3.5 MB
├─ SPIFFS:        2 MB
└─ Reserved:      4 KB
```

### Optimierungen für XIAO

1. **Compact Board Support**
   - Minimale externe Komponenten
   - Direkte USB Power
   - Integrated USB-to-Serial

2. **Dual-Core Optimization**
   - Core 0: Display & UI
   - Core 1: Audio & Analysis

3. **Memory Efficient**
   - FFT_SIZE: 2048 (reduziert von 4096)
   - Max 100 Messpunkte (statt 500)
   - Streaming statt Buffering

4. **Power Efficient**
   - Dynamic clock scaling
   - BLE update rate: 2 Hz (statt 10 Hz)
   - Display refresh: 30 FPS (statt 60)

### Troubleshooting

**Problem: "Failed to open port"**
```bash
# Lösung: XIAO in DFU-Mode bringen
# 1. Press Boot Button twice quickly
# 2. LED blinks (DFU mode)
# 3. pio run -t upload
```

**Problem: "Memory overflow"**
```
Solution:
1. Disable DEBUG_LEVEL (set to 1)
2. Reduce FFT_SIZE to 1024
3. Limit max_measurements to 50
```

**Problem: "Display not working"**
```
1. Check GPIO assignments in config.h
2. Verify SPI bus not shared incorrectly
3. Test with simple pin blink
4. Check display backlight brightness
```

### Schaltplan-Dateien

Download:
- `docs/seeed_xiao_esp32s3_schematic.pdf`
- `docs/display_wiring_diagram.pdf`
- `docs/audio_input_circuit.pdf`

### Referenzen

- Seeed XIAO ESP32-S3: https://wiki.seeedstudio.com/xiao_esp32s3_getting_started/
- Datasheet: https://files.seeedstudio.com/wiki_en/seeed_xiao_esp32s3_datasheet.pdf
- Pinout: https://wiki.seeedstudio.com/xiao_esp32s3_getting_started/#pinout

---

**Board: Seeed XIAO ESP32-S3**  
**Version:** 3.0  
**Last Updated:** 2026-09-05

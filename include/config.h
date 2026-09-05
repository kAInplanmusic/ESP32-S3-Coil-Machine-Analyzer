#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ============================================================================
// BOARD SELECTION - Seeed XIAO ESP32-S3
// ============================================================================
#define BOARD_SEEED_XIAO_ESP32S3

// ============================================================================
// PIN CONFIGURATION - Seeed XIAO ESP32-S3
// ============================================================================
// Seeed XIAO ESP32-S3 Pinout (Very Compact - 22-pin breakout)

// Display (SPI Interface) - ILI9488 or ST7796
#define TFT_MOSI    GPIO_NUM_8      // D8 - SPI MOSI
#define TFT_MISO    GPIO_NUM_9      // D9 - SPI MISO
#define TFT_SCLK    GPIO_NUM_7      // D7 - SPI Clock
#define TFT_CS      GPIO_NUM_6      // D6 - Chip Select
#define TFT_DC      GPIO_NUM_5      // D5 - Data/Command
#define TFT_RST     GPIO_NUM_4      // D4 - Reset
#define TFT_BL      GPIO_NUM_3      // D3 - Backlight (PWM)

// I2S Audio Input (Microphone)
#define I2S_NUM     I2S_NUM_0
#define I2S_BCLK    GPIO_NUM_42     // GPIO42 - Bit Clock
#define I2S_WS      GPIO_NUM_41     // GPIO41 - Word Select (LR Clock)
#define I2S_DIN     GPIO_NUM_40     // GPIO40 - Data In
#define I2S_DOUT    GPIO_NUM_2      // D2 - Data Out (optional)

// I2C Sensors (MPU6050 optional)
#define I2C_SDA     GPIO_NUM_1      // D1 - I2C Data
#define I2C_SCL     GPIO_NUM_0      // D0 - I2C Clock

// SD Card (SPI Interface)
#define SD_CS       GPIO_NUM_10     // D10 - Chip Select (external)
#define SD_MOSI     TFT_MOSI        // Shared with display
#define SD_MISO     TFT_MISO        // Shared with display
#define SD_SCLK     TFT_SCLK        // Shared with display

// USB Serial (CDC) - Automatic on XIAO
#define USB_TX      GPIO_NUM_20     // USB DP
#define USB_RX      GPIO_NUM_19     // USB DM

// Status LED (optional)
#define STATUS_LED  GPIO_NUM_11     // GPIO11 - Status indicator

// Button (optional)
#define BUTTON_PIN  GPIO_NUM_21     // GPIO21 - User button

// ============================================================================
// DISPLAY CONFIGURATION
// ============================================================================
#define DISPLAY_WIDTH           480
#define DISPLAY_HEIGHT          320
#define DISPLAY_ROTATION        1   // 0=Portrait, 1=Landscape
#define DISPLAY_SPI_FREQUENCY   40000000  // 40 MHz for XIAO
#define BACKLIGHT_INTENSITY     255
#define BACKLIGHT_PWM_CHANNEL   0
#define BACKLIGHT_PWM_FREQ      5000

// ============================================================================
// AUDIO CONFIGURATION
// ============================================================================
#define AUDIO_SAMPLE_RATE       44100   // 44.1 kHz
#define AUDIO_BITS_PER_SAMPLE   16      // 16-bit
#define AUDIO_CHANNELS          1       // Mono
#define AUDIO_BUFFER_SIZE       4410    // 100ms @ 44.1kHz
#define AUDIO_DMA_BUFFER_COUNT  2       // Double buffering

// Microphone gain settings
#define MICROPHONE_GAIN_MIN     -20.0f  // dB
#define MICROPHONE_GAIN_MAX     20.0f   // dB
#define MICROPHONE_GAIN_DEFAULT 0.0f    // dB

// ============================================================================
// FFT CONFIGURATION
// ============================================================================
#define FFT_SIZE                2048    // 2048-point FFT
#define FFT_WINDOW_TYPE         HANN    // HANN, HAMMING, BLACKMAN
#define FFT_SAMPLE_INTERVAL_MS  50      // New FFT every 50ms

// ============================================================================
// MEASUREMENT CONFIGURATION
// ============================================================================
#define MIN_CPS                 20      // 20 CPS (1200 CPM)
#define MAX_CPS                 200     // 200 CPS (12000 CPM)
#define DEFAULT_TARGET_CPS      85      // Default target

#define MIN_MEASUREMENTS        5       // Minimum samples
#define MAX_MEASUREMENTS        100     // Maximum samples
#define DEFAULT_MEASUREMENTS    50      // Default

// ============================================================================
// WAVEFORM ANALYSIS
// ============================================================================
#define POINTS_PER_WAVEFORM     20      // Analysis points per impact
#define MAX_HARMONICS           5       // H1 through H5
#define DECAY_ANALYSIS_POINTS   10      // Points for decay curve

// Peak detection sensitivity
#define PEAK_THRESHOLD_DB       -20.0f  // Minimum peak level
#define PEAK_MIN_DISTANCE_MS    5.0f    // Min distance between peaks

// ============================================================================
// QUALITY METRICS
// ============================================================================
#define CONSISTENCY_WEIGHT      0.30f   // Period consistency importance
#define HARMONIC_WEIGHT         0.25f   // Harmonic purity importance
#define DECAY_WEIGHT            0.20f   // Decay regularity importance
#define PEAK_WEIGHT             0.25f   // Peak amplitude importance

// ============================================================================
// STORAGE & LOGGING
// ============================================================================
#define EEPROM_SIZE             4096    // XIAO has 4KB EEPROM
#define CALIB_EEPROM_ADDR       0       // Calibration data start
#define SD_CARD_ENABLED         1       // Enable SD logging
#define MAX_LOG_FILES           50      // Keep up to 50 log files

// ============================================================================
// BLUETOOTH CONFIGURATION
// ============================================================================
#define BLE_ENABLED             1       // Enable Bluetooth LE
#define BLE_DEVICE_NAME         "CoilAnalyzer-XIAO"
#define BLE_UPDATE_INTERVAL_MS  500     // Update rate (2 Hz)

// ============================================================================
// SENSOR CONFIGURATION (Optional MPU6050)
// ============================================================================
#define MPU6050_ENABLED         0       // Disable by default (saves space)
#define MPU6050_I2C_ADDR        0x68    // Default I2C address
#define MPU6050_ACCEL_RANGE     2       // ±2g
#define MPU6050_GYRO_RANGE      250     // ±250°/s

// ============================================================================
// PERFORMANCE & DEBUG
// ============================================================================
#define DEBUG_SERIAL            1       // Enable serial debug output
#define DEBUG_LEVEL             2       // 0=None, 1=Errors, 2=Warnings, 3=Info, 4=Debug
#define PROFILE_TIMING          0       // Enable performance profiling
#define MEMORY_MONITOR          1       // Monitor free heap

// ============================================================================
// UI CONFIGURATION
// ============================================================================
#define UI_THEME_COLOR          0x1F    // Color scheme (0-31)
#define UI_FONT_SIZE            2       // Font multiplier
#define UI_UPDATE_RATE_MS       100     // UI refresh rate
#define UI_SHOW_GRID            1       // Show grid on plots

// ============================================================================
// XIAO-SPECIFIC OPTIMIZATIONS
// ============================================================================
#define XIAO_DUAL_CORE          1       // Use both cores
#define XIAO_PSRAM              0       // No PSRAM on XIAO
#define XIAO_FLASH_SIZE         16      // 16 MB flash
#define XIAO_COMPACT_MODE       1       // Optimize for compact layout

// ============================================================================
// FEATURE FLAGS
// ============================================================================
#define FEATURE_CALIBRATION     1       // Advanced calibration
#define FEATURE_SD_LOGGING      1       // SD card export
#define FEATURE_BLUETOOTH       1       // BLE interface
#define FEATURE_HARMONICS       1       // Harmonic analysis
#define FEATURE_DECAY_ANALYSIS  1       // Decay rate calculation
#define FEATURE_QUALITY_SCORE   1       // Quality scoring
#define FEATURE_REAL_TIME_PLOT  1       // Live waveform plotting
#define FEATURE_STATISTICS      1       // Statistics calculation

#endif // CONFIG_H

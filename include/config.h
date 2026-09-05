#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ============================================================================
// HARDWARE CONFIGURATION
// ============================================================================

// Display Configuration (ILI9488 or ST7796)
#define DISPLAY_WIDTH       480
#define DISPLAY_HEIGHT      320
#define DISPLAY_ROTATION    1
#define TFT_CS              5
#define TFT_DC              6
#define TFT_RST             7
#define TFT_MOSI            11
#define TFT_CLK             12
#define TFT_MISO            13
#define TFT_BACKLIGHT       8

// I2S Audio Input (Microphone)
#define I2S_PORT            I2S_NUM_0
#define I2S_SCK_PIN         3
#define I2S_WS_PIN          4
#define I2S_SD_PIN          2
#define I2S_SAMPLE_RATE     44100  // Hz
#define I2S_BIT_WIDTH       16     // bits
#define I2S_CHANNEL_FMT     I2S_CHANNEL_MONO
#define I2S_DMA_BUFF_LEN    256    // per buffer
#define I2S_DMA_BUFF_COUNT  8      // number of buffers

// Touch Screen (if available)
#define TOUCH_CS            10
#define TOUCH_IRQ           9

// SD Card (SPI)
#define SD_CS               1

// Optional: MPU6050 Vibration Sensor
#define MPU6050_SDA         20
#define MPU6050_SCL         21
#define MPU6050_ADDR        0x68

// ============================================================================
// SIGNAL PROCESSING CONFIGURATION
// ============================================================================

// FFT Parameters
#define FFT_SIZE            2048        // 2^11, sufficient for 44.1kHz @ 46ms window
#define FFT_BIN_COUNT       (FFT_SIZE / 2)
#define FFT_WINDOW_TYPE     HANN        // HANN, BLACKMAN_HARRIS, HAMMING

// Sampling & Buffering
#define SAMPLE_RATE         44100       // Hz (44.1 kHz standard audio)
#define SAMPLES_PER_FRAME   1024        // ~23ms @ 44.1kHz
#define RINGBUFFER_SIZE     (4 * FFT_SIZE)  // 8192 samples = ~186ms

// Frequency Bands
#define FREQ_MIN_BAND       100         // Hz - minimum frequency to analyze
#define FREQ_MAX_BAND       8000        // Hz - maximum frequency to analyze
#define FREQ_PEAK_MIN       400         // Hz - expected minimum impact frequency
#define FREQ_PEAK_MAX       4000        // Hz - expected maximum impact frequency

// Peak Detection
#define PEAK_THRESHOLD_DB   -30         // dB relative to maximum
#define PEAK_MIN_DISTANCE   (SAMPLE_RATE / 20)  // minimum 50ms between peaks
#define PEAK_HYSTERESIS     1.1         // ratio for peak detection hysteresis

// ============================================================================
// MEASUREMENT PARAMETERS
// ============================================================================

// CPS Range
#define CPS_MIN             0.33        // ~20 BPM (0.33 CPS)
#define CPS_MAX             3.33        // ~200 BPM (3.33 CPS)
#define CPS_SCALE_PERCENT   100         // 0-100% for display

// Quality Analysis
#define QUALITY_HISTORY_SIZE    100     // number of impacts to track
#define CONSISTENCY_THRESHOLD   15      // % standard deviation acceptable
#define HARMONIC_THRESHOLD      -20     // dB threshold for harmonics

// ============================================================================
// DISPLAY & UI CONFIGURATION
// ============================================================================

// Colors
#define COLOR_BLACK         0x0000
#define COLOR_WHITE         0xFFFF
#define COLOR_RED           0xF800
#define COLOR_GREEN         0x07E0
#define COLOR_BLUE          0x001F
#define COLOR_YELLOW        0xFFE0
#define COLOR_CYAN          0x07FF
#define COLOR_MAGENTA       0xF81F
#define COLOR_GRAY          0x8410
#define COLOR_DARK_GRAY     0x4208
#define COLOR_LIGHT_GRAY    0xC618

// UI Layout
#define UI_UPDATE_INTERVAL  100         // ms between UI refreshes
#define WAVEFORM_HEIGHT     120         // pixels for waveform display
#define SPECTRUM_HEIGHT     100         // pixels for spectrum display
#define FONT_SIZE_SMALL     1
#define FONT_SIZE_MEDIUM    2
#define FONT_SIZE_LARGE     3

// ============================================================================
// CALIBRATION & THRESHOLDS
// ============================================================================

// Microphone Calibration
#define MIC_REFERENCE_LEVEL 32767       // ADC mid-range for 16-bit
#define MIC_GAIN_DB         0           // Initial gain in dB
#define NOISE_FLOOR_DB      -60         // expected noise floor

// Impact Detection
#define IMPACT_THRESHOLD_DB -20         // dB above noise floor to detect impact
#define IMPACT_DURATION_MIN 5           // ms - minimum impact duration
#define IMPACT_DURATION_MAX 200         // ms - maximum impact duration

// ============================================================================
// LOGGING & DATA STORAGE
// ============================================================================

#define LOG_SERIAL          true        // Enable serial debug logging
#define LOG_SD_CARD         true        // Enable SD card logging
#define LOG_CSV_EXPORT      true        // Enable CSV data export

#define CSV_LOG_PATH        "/logs/"
#define CSV_LOG_INTERVAL    1000        // ms between CSV entries
#define CSV_BUFFER_SIZE     1024        // bytes

// ============================================================================
// MEMORY OPTIMIZATION
// ============================================================================

#define USE_PSRAM           true        // Use external PSRAM for large buffers
#define OPTIMIZE_FOR_SPEED  true        // Disable some safety checks for speed

#endif // CONFIG_H

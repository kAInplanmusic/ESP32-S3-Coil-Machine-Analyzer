#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ==================== SYSTEM CONFIGURATION ====================

#define LOG_SERIAL              true   // Enable serial debug output
#define SERIAL_BAUD_RATE        115200 // Serial communication speed

// ==================== AUDIO INPUT (I2S) ====================

// I2S Pins (ESP32-S3)
#define I2S_SCK_PIN             3      // Serial Clock
#define I2S_WS_PIN              4      // Word Select
#define I2S_SD_PIN              2      // Serial Data
#define I2S_PORT                I2S_NUM_0

// Audio Parameters
#define SAMPLE_RATE             44100  // Hz (44.1 kHz)
#define AUDIO_CHANNELS          1      // Mono
#define BITS_PER_SAMPLE         16     // 16-bit samples
#define DMA_BUFFER_LEN          256    // DMA buffer length
#define DMA_BUFFER_COUNT        2      // Number of DMA buffers

// ==================== SIGNAL PROCESSING (FFT) ====================

#define FFT_SIZE                2048   // FFT points (46ms window @ 44.1kHz)
#define FREQ_MIN_BAND           100    // Hz (low frequency cutoff)
#define FREQ_MAX_BAND           8000   // Hz (high frequency cutoff)
#define FREQ_PEAK_MIN           400    // Hz (expected impact freq min)
#define FREQ_PEAK_MAX           4000   // Hz (expected impact freq max)

// ==================== PEAK DETECTION ====================

#define PEAK_THRESHOLD_DB       -30    // dB (detection threshold)
#define PEAK_MIN_DISTANCE       2205   // samples (~50ms @ 44.1kHz)
#define PEAK_HISTORY_SIZE       100    // Number of peaks to track

// ==================== CPS MEASUREMENT (0-250 CPS) ====================

#define CPS_MIN                 0.0f   // Cycles per second (minimum)
#define CPS_MAX                 250.0f // Cycles per second (maximum)
#define CPS_MEASUREMENT_TIME    2.0f   // Seconds (window for CPS calc)

// ==================== IMPACT ANALYSIS (3-POINT METHOD) ====================

// 3-Point Analysis:
// - Point C: Start point (impulse trigger/rising edge)
// - Point A: Ausschlagpunkt (peak/maximum amplitude)
// - Point B: Wechselpunkt (zero crossing or direction change)

#define IMPACT_MEASURE_COUNT    100    // Collect 100 impacts per measurement
#define IMPACT_TIMEOUT_MS       10000  // Timeout if no impact (ms)
#define JITTER_MAX_ACCEPTABLE   0.5f   // Maximum acceptable jitter (ms)

// ==================== QUALITY SCORING ====================

// Quality Color Mapping:
// RED:    0-60%   (poor consistency)
// ORANGE: 61-74%  (fair consistency)
// YELLOW: 75-85%  (good consistency)
// GREEN:  86-94%  (excellent consistency)
// BRIGHT GREEN with black text: 95-100% (perfect)

#define QUALITY_CONSISTENCY_WEIGHT   0.6f  // 60% weight to consistency
#define QUALITY_SIGNAL_WEIGHT        0.4f  // 40% weight to signal strength
#define QUALITY_SIGNAL_THRESHOLD     0.3f  // Amplitude for 100% signal score

// ==================== DISPLAY (TFT) ====================

// Display Pins (ESP32-S3 with ILI9488/ST7796)
#define TFT_CS_PIN              5      // Chip Select
#define TFT_DC_PIN              6      // Data/Command
#define TFT_RST_PIN             7      // Reset
#define TFT_MOSI_PIN            11     // MOSI (SPI)
#define TFT_SCLK_PIN            12     // Clock (SPI)
#define TFT_MISO_PIN            13     // MISO (SPI)
#define TFT_BACKLIGHT_PIN       8      // Backlight (PWM)

// Display Parameters
#define DISPLAY_WIDTH           480    // pixels
#define DISPLAY_HEIGHT          320    // pixels
#define DISPLAY_ROTATION        1      // 0-3 (landscape)
#define DISPLAY_USE_DMA         true   // Use DMA for faster updates

// Display Colors (RGB565)
#define COLOR_BLACK             0x0000
#define COLOR_WHITE             0xFFFF
#define COLOR_RED               0xF800
#define COLOR_GREEN             0x07E0
#define COLOR_BLUE              0x001F
#define COLOR_YELLOW            0xFFE0
#define COLOR_ORANGE            0xFD20  // Orange (approx)
#define COLOR_CYAN              0x07FF
#define COLOR_GRAY              0x8410
#define COLOR_DARK_GRAY         0x4208
#define COLOR_LIGHT_GRAY        0xC618

// Font Sizes
#define FONT_SIZE_SMALL         1      // 8x5 pixels
#define FONT_SIZE_MEDIUM        2      // 16x10 pixels
#define FONT_SIZE_LARGE         3      // 24x15 pixels

// Display Updates
#define UI_UPDATE_INTERVAL      100    // ms (10 Hz update rate)
#define WAVEFORM_HEIGHT         80     // pixels (in combined view)
#define SPECTRUM_HEIGHT         80     // pixels (in combined view)

// ==================== MICROPHONE POSITION ====================

// Default Microphone Position Configuration
#define MICROPHONE_DISTANCE_MM  100.0f // Default distance (mm)
#define MICROPHONE_ANGLE_DEG    0.0f   // Default angle (degrees from vertical)

// Valid Ranges
#define MICROPHONE_MIN_DISTANCE 10.0f  // Minimum distance (mm)
#define MICROPHONE_MAX_DISTANCE 500.0f // Maximum distance (mm)

// ==================== MEASUREMENT UI ====================

// START Button Configuration
#define START_BUTTON_X          150    // Button X position
#define START_BUTTON_Y          240    // Button Y position
#define START_BUTTON_WIDTH      180    // Button width (pixels)
#define START_BUTTON_HEIGHT     50     // Button height (pixels)

// Countdown Display
#define COUNTDOWN_TARGET        100    // Measure 100 impacts

// ==================== MEMORY & PERFORMANCE ====================

// Buffer Sizes
#define SAMPLE_BUFFER_SIZE      4096   // Audio sample ring buffer
#define FFT_OUTPUT_SIZE         (FFT_SIZE / 2)  // Spectrum bins
#define HISTORY_BUFFER_SIZE     1000   // Impact history size

// Task Stack Sizes
#define AUDIO_TASK_STACK_SIZE   4096   // bytes
#define PROCESSING_TASK_STACK   3072   // bytes
#define DISPLAY_TASK_STACK_SIZE 2048   // bytes
#define UI_TASK_STACK_SIZE      2048   // bytes

// ==================== ADVANCED SIGNAL PROCESSING ====================

// Windowing Function for FFT
#define WINDOW_FUNCTION         WINDOW_HANN  // Hann or Blackman-Harris
#define WINDOW_HANN             1
#define WINDOW_BLACKMAN_HARRIS  2

// Filter Coefficients
#define FILTER_ORDER            4      // Butterworth filter order
#define FILTER_CUTOFF_LOW       100    // Hz
#define FILTER_CUTOFF_HIGH      8000   // Hz

// RMS & Energy Calculation
#define RMS_WINDOW_SIZE         2048   // samples
#define ENERGY_THRESHOLD_DB     -40    // dB (noise floor)

// ==================== LOGGING & CALIBRATION ====================

#define LOG_SD_CARD             false  // Enable SD card logging (future)
#define CALIBRATION_SAMPLES     2048   // Samples for calibration
#define CALIBRATION_TIMEOUT_MS  5000   // Timeout for calibration

// ==================== DERIVATIVE CONFIGURATIONS ====================

// Calculated Values (don't modify)
#define SAMPLES_PER_MS          (SAMPLE_RATE / 1000)  // 44 samples/ms
#define MS_PER_FRAME            (1000.0f / SAMPLE_RATE * FFT_SIZE)  // ms per FFT frame
#define FFT_FREQ_RESOLUTION     (SAMPLE_RATE / FFT_SIZE)  // Hz per bin

#endif // CONFIG_H

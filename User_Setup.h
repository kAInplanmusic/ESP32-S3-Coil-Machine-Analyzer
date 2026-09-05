# TFT_eSPI User Setup Configuration
# Configure for ESP32-S3 with ILI9488/ST7796 Display

# Uncomment for ILI9488 display
#define ILI9488_DRIVER

# Uncomment for ST7796 display
#define ST7796_DRIVER

# Display width and height
#define TFT_WIDTH  480
#define TFT_HEIGHT 320

# Pin definitions for ESP32-S3
#define TFT_CS     5   // Chip select
#define TFT_DC     6   // Data/Command
#define TFT_RST    7   // Reset
#define TFT_MOSI   11  // MOSI
#define TFT_SCLK   12  // Clock
#define TFT_MISO   13  // MISO

# SPI speed (MHz)
#define SPI_FREQUENCY  40

# Rotation
#define TFT_ROTATION   1  // 0-3

# Color order
#define TFT_RGB_ORDER TFT_BGR

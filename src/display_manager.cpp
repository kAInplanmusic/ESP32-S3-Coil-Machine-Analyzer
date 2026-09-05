#include "display_manager.h"
#include <esp_log.h>

static const char* TAG = "DisplayManager";

DisplayManager::DisplayManager()
    : _viewMode(VIEW_COMBINED), _waveformBuffer(nullptr), 
      _spectrumBuffer(nullptr), _lastUpdateTime(0) {
}

DisplayManager::~DisplayManager() {
    if (_waveformBuffer) free(_waveformBuffer);
    if (_spectrumBuffer) free(_spectrumBuffer);
}

bool DisplayManager::init() {
    ESP_LOGI(TAG, "Initializing TFT display...");
    
    _tft.init();
    _tft.setRotation(DISPLAY_ROTATION);
    _tft.fillScreen(COLOR_BLACK);
    
    // Allocate display buffers
    _waveformBuffer = (uint16_t*)malloc(DISPLAY_WIDTH * WAVEFORM_HEIGHT * sizeof(uint16_t));
    _spectrumBuffer = (uint16_t*)malloc(DISPLAY_WIDTH * SPECTRUM_HEIGHT * sizeof(uint16_t));
    
    if (!_waveformBuffer || !_spectrumBuffer) {
        ESP_LOGE(TAG, "Failed to allocate display buffers");
        return false;
    }
    
    // Set backlight
    pinMode(TFT_BACKLIGHT, OUTPUT);
    setBacklight(255);
    
    ESP_LOGI(TAG, "Display initialized successfully");
    return true;
}

void DisplayManager::update(const SignalProcessor& processor, float cps, uint8_t qualityScore) {
    unsigned long current_time = millis();
    
    // Throttle updates to UI_UPDATE_INTERVAL
    if (current_time - _lastUpdateTime < UI_UPDATE_INTERVAL) {
        return;
    }
    _lastUpdateTime = current_time;
    
    switch (_viewMode) {
        case VIEW_COMBINED:
            drawCombinedView(processor, cps, qualityScore);
            break;
        case VIEW_WAVEFORM:
            drawWaveform(processor, 10, 30, DISPLAY_WIDTH - 20, DISPLAY_HEIGHT - 40);
            break;
        case VIEW_SPECTRUM:
            drawSpectrum(processor.getSpectrumData(), 10, 30, DISPLAY_WIDTH - 20, DISPLAY_HEIGHT - 40);
            break;
        case VIEW_STATS: {
            auto stats = processor.getImpactIntervalStats();
            drawStatistics(processor, stats, 10, 30);
            break;
        }
    }
}

void DisplayManager::setViewMode(ViewMode mode) {
    if (_viewMode != mode) {
        _viewMode = mode;
        _tft.fillScreen(COLOR_BLACK);
        ESP_LOGI(TAG, "View mode changed to: %d", mode);
    }
}

void DisplayManager::clear() {
    _tft.fillScreen(COLOR_BLACK);
}

void DisplayManager::setBacklight(uint8_t brightness) {
    // PWM control for backlight (0-255)
    analogWrite(TFT_BACKLIGHT, brightness);
}

void DisplayManager::drawInitScreen() {
    _tft.fillScreen(COLOR_BLACK);
    _tft.setTextColor(COLOR_WHITE, COLOR_BLACK);
    _tft.setTextSize(FONT_SIZE_LARGE);
    _tft.setCursor(50, 100);
    _tft.println("ESP32-S3");
    _tft.println("Coil Machine Analyzer");
    _tft.setTextSize(FONT_SIZE_SMALL);
    _tft.setCursor(80, 200);
    _tft.println("Initializing...");
}

void DisplayManager::drawError(const char* title, const char* message) {
    _tft.fillScreen(COLOR_BLACK);
    _tft.setTextColor(COLOR_RED, COLOR_BLACK);
    _tft.setTextSize(FONT_SIZE_LARGE);
    _tft.setCursor(20, 50);
    _tft.println(title);
    _tft.setTextColor(COLOR_WHITE, COLOR_BLACK);
    _tft.setTextSize(FONT_SIZE_MEDIUM);
    _tft.setCursor(20, 150);
    _tft.println(message);
}

void DisplayManager::drawCombinedView(const SignalProcessor& processor, 
                                     float cps, uint8_t qualityScore) {
    // Draw status bar
    drawStatusBar("Coil Analyzer", "Running");
    
    // Draw waveform (top)
    drawWaveform(processor, 10, 25, DISPLAY_WIDTH - 20, 80);
    
    // Draw spectrum (middle)
    drawSpectrum(processor.getSpectrumData(), 10, 110, DISPLAY_WIDTH - 20, 80);
    
    // Draw CPS gauge and quality (bottom)
    drawCPSGauge(cps, qualityScore, 10, 195);
    
    // Draw stats
    auto stats = processor.getImpactIntervalStats();
    drawStatistics(processor, stats, 280, 195);
}

void DisplayManager::drawWaveform(const SignalProcessor& processor, 
                                  int x, int y, int width, int height) {
    // Get waveform data
    int16_t waveform[width];
    size_t samples = processor.getWaveformData(waveform, width);
    
    if (samples == 0) return;
    
    // Draw grid
    drawGrid(x, y, width, height, 4);
    
    // Draw waveform
    _tft.setTextColor(COLOR_GREEN);
    
    // Find min/max for scaling
    int16_t min_val = waveform[0], max_val = waveform[0];
    for (size_t i = 0; i < samples; i++) {
        if (waveform[i] < min_val) min_val = waveform[i];
        if (waveform[i] > max_val) max_val = waveform[i];
    }
    
    int16_t range = max_val - min_val;
    if (range == 0) range = 1;
    
    // Draw line segments
    for (size_t i = 1; i < samples && i < (size_t)width; i++) {
        int y1 = y + height - (int)(((waveform[i-1] - min_val) * height) / range);
        int y2 = y + height - (int)(((waveform[i] - min_val) * height) / range);
        
        _tft.drawLine(x + i - 1, y1, x + i, y2, COLOR_GREEN);
    }
}

void DisplayManager::drawSpectrum(const SignalProcessor::SpectrumData& spectrum,
                                 int x, int y, int width, int height) {
    if (spectrum.magnitude.empty()) return;
    
    // Draw grid
    drawGrid(x, y, width, height, 4);
    
    // Draw frequency scale
    drawFrequencyScale(x, y + height, width, FREQ_MIN_BAND, FREQ_MAX_BAND);
    
    // Find display range
    float max_mag = *std::max_element(spectrum.magnitude.begin(), spectrum.magnitude.end());
    float min_mag = max_mag - 60.0f;  // 60 dB range
    
    // Draw spectrum bars
    _tft.setTextColor(COLOR_YELLOW);
    
    size_t bins_to_display = std::min((size_t)width, spectrum.magnitude.size());
    for (size_t i = 0; i < bins_to_display; i++) {
        float mag = spectrum.magnitude[i * spectrum.magnitude.size() / bins_to_display];
        float normalized = (mag - min_mag) / (max_mag - min_mag);
        normalized = std::max(0.0f, std::min(1.0f, normalized));
        
        int bar_height = (int)(normalized * height);
        _tft.drawFastVLine(x + i, y + height - bar_height, bar_height, COLOR_YELLOW);
    }
}

void DisplayManager::drawCPSGauge(float cps, uint8_t qualityScore, int x, int y) {
    // Draw CPS value large
    _tft.setTextColor(COLOR_WHITE, COLOR_BLACK);
    _tft.setTextSize(FONT_SIZE_LARGE);
    _tft.setCursor(x, y);
    _tft.print("CPS: ");
    _tft.setTextColor(COLOR_CYAN);
    _tft.printf("%.2f", cps);
    
    // Draw percentage
    float percent = (cps / CPS_MAX) * 100.0f;
    percent = std::min(100.0f, percent);
    
    _tft.setTextColor(COLOR_WHITE);
    _tft.setCursor(x, y + 25);
    _tft.printf("%.0f%%", percent);
    
    // Draw quality bar
    drawQualityBar(qualityScore, x, y + 50, 120, 15);
}

void DisplayManager::drawQualityBar(uint8_t score, int x, int y, int width, int height) {
    // Background
    _tft.fillRect(x, y, width, height, COLOR_DARK_GRAY);
    
    // Colored bar
    uint16_t color;
    if (score > 75) {
        color = COLOR_GREEN;
    } else if (score > 50) {
        color = COLOR_YELLOW;
    } else {
        color = COLOR_RED;
    }
    
    int bar_width = (width * score) / 100;
    _tft.fillRect(x, y, bar_width, height, color);
    
    // Border
    _tft.drawRect(x, y, width, height, COLOR_WHITE);
    
    // Label and value
    _tft.setTextColor(COLOR_WHITE);
    _tft.setTextSize(FONT_SIZE_SMALL);
    _tft.setCursor(x + 5, y + 2);
    _tft.printf("Quality: %d%%", score);
}

void DisplayManager::drawStatistics(const SignalProcessor& processor,
                                   const std::vector<float>& stats, int x, int y) {
    _tft.setTextColor(COLOR_WHITE, COLOR_BLACK);
    _tft.setTextSize(FONT_SIZE_SMALL);
    
    _tft.setCursor(x, y);
    _tft.println("Statistics:");
    
    if (stats.size() >= 4) {
        _tft.printf("Mean: %.1f ms\n", stats[0]);
        _tft.printf("StdDev: %.1f ms\n", stats[1]);
        _tft.printf("Min: %.1f ms\n", stats[2]);
        _tft.printf("Max: %.1f ms\n", stats[3]);
    }
    
    _tft.printf("RMS: %.3f\n", processor.getRMSEnergy());
    _tft.printf("Peak-Peak: %.0f\n", processor.getPeakToPeak());
    _tft.printf("Noise: %.1f dB\n", processor.getNoiseFloor());
}

void DisplayManager::drawTextBox(int x, int y, const char* text, 
                                uint16_t textColor, uint16_t bgColor) {
    // Simple implementation - just draw text with background color
    _tft.setTextColor(textColor, bgColor);
    _tft.setCursor(x, y);
    _tft.println(text);
}

void DisplayManager::drawNumber(float value, int decimals, const char* unit,
                               int x, int y, uint8_t fontSize, uint16_t color) {
    _tft.setTextColor(color);
    _tft.setTextSize(fontSize);
    _tft.setCursor(x, y);
    _tft.printf("%.*f %s", decimals, value, unit);
}

void DisplayManager::drawGrid(int x, int y, int width, int height, int divisions) {
    _tft.setTextColor(COLOR_DARK_GRAY);
    
    // Vertical lines
    for (int i = 0; i <= divisions; i++) {
        int px = x + (width * i) / divisions;
        _tft.drawFastVLine(px, y, height, COLOR_DARK_GRAY);
    }
    
    // Horizontal lines
    for (int i = 0; i <= divisions; i++) {
        int py = y + (height * i) / divisions;
        _tft.drawFastHLine(x, py, width, COLOR_DARK_GRAY);
    }
}

void DisplayManager::drawFrequencyScale(int x, int y, int width, 
                                       float freqMin, float freqMax) {
    _tft.setTextColor(COLOR_LIGHT_GRAY);
    _tft.setTextSize(FONT_SIZE_SMALL);
    
    // Draw frequency labels
    const int num_labels = 4;
    for (int i = 0; i < num_labels; i++) {
        float freq = freqMin + (freqMax - freqMin) * i / (num_labels - 1);
        int px = x + (width * i) / (num_labels - 1);
        
        _tft.setCursor(px - 10, y);
        _tft.printf("%.0fHz", freq);
    }
}

uint16_t DisplayManager::interpolateColor(float value, uint16_t minColor, uint16_t maxColor) {
    value = std::max(0.0f, std::min(1.0f, value));
    
    // Extract RGB components
    uint8_t r1 = (minColor >> 11) & 0x1F;
    uint8_t g1 = (minColor >> 5) & 0x3F;
    uint8_t b1 = minColor & 0x1F;
    
    uint8_t r2 = (maxColor >> 11) & 0x1F;
    uint8_t g2 = (maxColor >> 5) & 0x3F;
    uint8_t b2 = maxColor & 0x1F;
    
    // Interpolate
    uint8_t r = r1 + (r2 - r1) * value;
    uint8_t g = g1 + (g2 - g1) * value;
    uint8_t b = b1 + (b2 - b1) * value;
    
    // Recombine
    return ((r & 0x1F) << 11) | ((g & 0x3F) << 5) | (b & 0x1F);
}

void DisplayManager::drawStatusBar(const char* title, const char* status) {
    // Draw background bar
    _tft.fillRect(0, 0, DISPLAY_WIDTH, 20, COLOR_GRAY);
    
    // Draw title and status
    _tft.setTextColor(COLOR_WHITE, COLOR_GRAY);
    _tft.setTextSize(FONT_SIZE_SMALL);
    _tft.setCursor(5, 5);
    _tft.print(title);
    
    _tft.setTextColor(COLOR_GREEN, COLOR_GRAY);
    _tft.setCursor(DISPLAY_WIDTH - 60, 5);
    _tft.print(status);
}

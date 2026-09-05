#include "measurement_ui.h"
#include <esp_log.h>

static const char* TAG = "MeasurementUI";

MeasurementUI::MeasurementUI()
    : _display(nullptr), _analyzer(nullptr), _state(STATE_IDLE),
      _remaining_impacts(100), _last_update_time(0) {
}

MeasurementUI::~MeasurementUI() {
}

bool MeasurementUI::init(DisplayManager* display, ImpactAnalyzer* analyzer) {
    _display = display;
    _analyzer = analyzer;
    _state = STATE_IDLE;
    _remaining_impacts = 100;
    
    ESP_LOGI(TAG, "Measurement UI initialized");
    return true;
}

void MeasurementUI::update() {
    unsigned long current_time = millis();
    if (current_time - _last_update_time < UI_UPDATE_INTERVAL) {
        return;
    }
    _last_update_time = current_time;
    
    switch (_state) {
        case STATE_IDLE:
            drawIdleScreen();
            break;
        case STATE_MEASURING:
            drawMeasuringScreen();
            break;
        case STATE_COMPLETE:
            drawResultsScreen();
            break;
        case STATE_ERROR:
            drawErrorScreen("Measurement Error");
            break;
    }
}

void MeasurementUI::handleStartButton() {
    if (_state == STATE_IDLE) {
        _analyzer->startMeasurement();
        setState(STATE_MEASURING);
        _remaining_impacts = 100;
        ESP_LOGI(TAG, "Measurement started by user");
    } else if (_state == STATE_COMPLETE) {
        setState(STATE_IDLE);
        _remaining_impacts = 100;
        _analyzer->reset();
        ESP_LOGI(TAG, "Returned to idle state");
    }
}

void MeasurementUI::drawIdleScreen() {
    _display->clear();
    
    // Title
    _display->setTextColor(COLOR_WHITE);
    _display->setTextSize(FONT_SIZE_LARGE);
    _display->setCursor(40, 20);
    _display->println("Coil Analyzer");
    
    // Microphone info
    float dist, angle;
    _analyzer->getMicrophonePosition(dist, angle);
    drawMicrophoneInfo(dist, angle, 20, 80);
    
    // Instructions
    _display->setTextColor(COLOR_CYAN);
    _display->setTextSize(FONT_SIZE_MEDIUM);
    _display->setCursor(30, 150);
    _display->println("Press START to begin");
    _display->setCursor(20, 180);
    _display->println("measuring 100 impacts");
    
    // START button
    drawStartButton(150, 240, false);
    
    // Footer info
    _display->setTextColor(COLOR_LIGHT_GRAY);
    _display->setTextSize(FONT_SIZE_SMALL);
    _display->setCursor(20, 300);
    _display->println("Range: 0-250 CPS");
}

void MeasurementUI::drawMeasuringScreen() {
    _display->clear();
    
    // Status bar
    _display->fillRect(0, 0, DISPLAY_WIDTH, 25, COLOR_GRAY);
    _display->setTextColor(COLOR_WHITE, COLOR_GRAY);
    _display->setTextSize(FONT_SIZE_SMALL);
    _display->setCursor(10, 5);
    _display->print("Measuring... ");
    _display->setTextColor(COLOR_GREEN, COLOR_GRAY);
    _display->printf("%d%%", _analyzer->getMeasurementProgress());
    
    // Large CPS display
    float cps = _analyzer->getCPS();
    uint8_t quality = _analyzer->getStats().quality_score;
    drawCPSValue(cps, quality, 40, 80, quality >= 95);
    
    // Countdown counter (large)
    _remaining_impacts = 100 - _analyzer->getMeasurementProgress();
    drawCountdown(_remaining_impacts, 180, 200);
    
    // Consistency indicator
    _display->setTextColor(COLOR_YELLOW);
    _display->setTextSize(FONT_SIZE_MEDIUM);
    _display->setCursor(50, 260);
    _display->printf("Consistency: %.0f%%", _analyzer->getConsistency());
}

void MeasurementUI::drawResultsScreen() {
    _display->clear();
    
    const auto& stats = _analyzer->getStats();
    uint16_t quality_color = _analyzer->getQualityColor();
    
    // If perfect quality (95-100%), use huge green display
    if (stats.quality_score >= 95) {
        // Full screen green background
        _display->fillScreen(COLOR_GREEN);
        
        // Huge CPS text in black
        _display->setTextColor(COLOR_BLACK, COLOR_GREEN);
        _display->setTextSize(3);  // Huge text
        _display->setCursor(80, 100);
        _display->printf("%.0f CPS", stats.cps_calculated);
        
        // Quality indicator
        _display->setTextColor(COLOR_WHITE, COLOR_GREEN);
        _display->setTextSize(FONT_SIZE_LARGE);
        _display->setCursor(100, 200);
        _display->println("PERFECT!");
        
        // Score
        _display->setTextSize(FONT_SIZE_MEDIUM);
        _display->setCursor(90, 250);
        _display->printf("Score: %d%%", stats.quality_score);
    } else {
        // Standard results screen with colored header
        _display->fillRect(0, 0, DISPLAY_WIDTH, 40, quality_color);
        
        // CPS on colored background
        _display->setTextColor(COLOR_BLACK, quality_color);
        _display->setTextSize(FONT_SIZE_LARGE);
        _display->setCursor(80, 10);
        _display->printf("%.2f CPS", stats.cps_calculated);
        
        // Results
        _display->setTextColor(COLOR_WHITE);
        _display->setTextSize(FONT_SIZE_MEDIUM);
        _display->setCursor(20, 60);
        _display->println("Measurement Complete");
        
        drawStatsPanel(stats, 20, 100);
    }
    
    // Reset button instruction
    _display->setTextColor(COLOR_CYAN);
    _display->setTextSize(FONT_SIZE_SMALL);
    _display->setCursor(30, 290);
    _display->println("Press START to reset");
}

void MeasurementUI::drawErrorScreen(const char* error_msg) {
    _display->clear();
    _display->fillRect(0, 0, DISPLAY_WIDTH, 40, COLOR_RED);
    
    _display->setTextColor(COLOR_WHITE, COLOR_RED);
    _display->setTextSize(FONT_SIZE_LARGE);
    _display->setCursor(50, 10);
    _display->println("ERROR");
    
    _display->setTextColor(COLOR_WHITE);
    _display->setTextSize(FONT_SIZE_MEDIUM);
    _display->setCursor(20, 80);
    _display->println(error_msg);
    
    _display->setTextSize(FONT_SIZE_SMALL);
    _display->setCursor(20, 150);
    _display->println("Press START to retry");
}

void MeasurementUI::setState(MeasurementState state) {
    _state = state;
}

void MeasurementUI::drawCPSValue(float cps, uint8_t quality, int x, int y, bool huge_mode) {
    uint16_t color = getQualityColorForValue(quality);
    
    if (huge_mode) {
        // Huge mode - giant numbers
        _display->setTextColor(COLOR_BLACK, COLOR_GREEN);
        _display->setTextSize(3);
    } else {
        _display->setTextColor(color);
        _display->setTextSize(FONT_SIZE_LARGE);
    }
    
    _display->setCursor(x, y);
    _display->printf("%.2f CPS", cps);
    
    // Quality below
    _display->setTextColor(COLOR_WHITE);
    _display->setTextSize(FONT_SIZE_MEDIUM);
    _display->setCursor(x, y + 50);
    _display->printf("Quality: %d%%", quality);
}

void MeasurementUI::drawCountdown(uint8_t remaining, int x, int y) {
    // Large countdown number
    _display->setTextColor(COLOR_CYAN);
    _display->setTextSize(FONT_SIZE_LARGE);
    _display->setCursor(x, y);
    _display->printf("%d", remaining);
    
    _display->setTextColor(COLOR_WHITE);
    _display->setTextSize(FONT_SIZE_MEDIUM);
    _display->setCursor(x, y + 40);
    _display->println("impacts left");
}

void MeasurementUI::drawStartButton(int x, int y, bool active) {
    uint16_t color = active ? COLOR_GREEN : COLOR_GRAY;
    uint16_t text_color = active ? COLOR_BLACK : COLOR_WHITE;
    
    // Draw button background
    _display->fillRect(x, y, 180, 50, color);
    _display->drawRect(x, y, 180, 50, COLOR_WHITE);
    
    // Draw text
    _display->setTextColor(text_color, color);
    _display->setTextSize(FONT_SIZE_LARGE);
    _display->setCursor(x + 50, y + 12);
    _display->println("START");
}

void MeasurementUI::drawMicrophoneInfo(float distance, float angle, int x, int y) {
    _display->setTextColor(COLOR_LIGHT_GRAY);
    _display->setTextSize(FONT_SIZE_SMALL);
    
    _display->setCursor(x, y);
    _display->println("Microphone Position:");
    
    _display->setCursor(x + 10, y + 20);
    _display->printf("Distance: %.0f mm", distance);
    
    _display->setCursor(x + 10, y + 35);
    _display->printf("Angle: %.0f°", angle);
}

void MeasurementUI::drawStatsPanel(const ImpactAnalyzer::MeasurementStats& stats, int x, int y) {
    _display->setTextColor(COLOR_WHITE);
    _display->setTextSize(FONT_SIZE_SMALL);
    
    _display->setCursor(x, y);
    _display->printf("Mean Period: %.2f ms\n", stats.mean_period_ms);
    
    _display->setCursor(x, y + 20);
    _display->printf("Consistency: %.1f%%\n", stats.consistency_percent);
    
    _display->setCursor(x, y + 40);
    _display->printf("Jitter A: %.3f ms\n", stats.jitter_A_ms);
    
    _display->setCursor(x, y + 60);
    _display->printf("Jitter B: %.3f ms\n", stats.jitter_B_ms);
    
    _display->setCursor(x, y + 80);
    _display->printf("Samples: %d", stats.samples_collected);
}

uint16_t MeasurementUI::getQualityColorForValue(uint8_t quality) const {
    if (quality >= 95) {
        return COLOR_GREEN;      // Bright green: 95-100% (perfect)
    } else if (quality >= 86) {
        return COLOR_GREEN;      // Green: 86-94% (excellent)
    } else if (quality >= 75) {
        return COLOR_YELLOW;     // Yellow: 75-85% (good)
    } else if (quality >= 61) {
        return COLOR_ORANGE;     // Orange: 61-74% (fair)
    } else {
        return COLOR_RED;        // Red: 0-60% (poor)
    }
}

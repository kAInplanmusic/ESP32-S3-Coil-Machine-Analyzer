#include "advanced_measurement_ui.h"
#include <esp_log.h>
#include <cmath>

static const char* TAG = "AdvancedMeasurementUI";

#define UI_UPDATE_INTERVAL 100  // ms

AdvancedMeasurementUI::AdvancedMeasurementUI()
    : _display(nullptr), _analyzer(nullptr), _state(STATE_IDLE),
      _last_update_time(0), _current_view_index(0), _selected_point(0) {
}

AdvancedMeasurementUI::~AdvancedMeasurementUI() {
}

bool AdvancedMeasurementUI::init(DisplayManager* display, AdvancedImpactAnalyzer* analyzer) {
    _display = display;
    _analyzer = analyzer;
    _state = STATE_IDLE;
    
    ESP_LOGI(TAG, "Advanced Measurement UI initialized");
    return true;
}

void AdvancedMeasurementUI::update() {
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
        case STATE_WAVEFORM_LIVE:
            drawWaveformLiveView();
            break;
        case STATE_POINTS_TABLE:
            drawPointsTableView();
            break;
        case STATE_HARMONICS:
            drawHarmonicsView();
            break;
        case STATE_DECAY_ANALYSIS:
            drawDecayAnalysisView();
            break;
        case STATE_QUALITY_BREAKDOWN:
            drawQualityBreakdownView();
            break;
        case STATE_COMPARISON:
            drawComparisonView();
            break;
        case STATE_ERROR:
            drawErrorScreen("Measurement Error");
            break;
    }
}

void AdvancedMeasurementUI::handleStartButton() {
    if (_state == STATE_IDLE) {
        _analyzer->startMeasurement();
        setState(STATE_MEASURING);
        ESP_LOGI(TAG, "Measurement started");
    } else if (_state == STATE_QUALITY_BREAKDOWN || _state == STATE_WAVEFORM_LIVE) {
        setState(STATE_IDLE);
        _analyzer->reset();
        ESP_LOGI(TAG, "Reset to idle");
    }
}

void AdvancedMeasurementUI::handleNavigate(int8_t direction) {
    if (direction == 1) {
        // Next view
        _current_view_index = (_current_view_index + 1) % 5;
        switch (_current_view_index) {
            case 0: setState(STATE_WAVEFORM_LIVE); break;
            case 1: setState(STATE_POINTS_TABLE); break;
            case 2: setState(STATE_HARMONICS); break;
            case 3: setState(STATE_DECAY_ANALYSIS); break;
            case 4: setState(STATE_QUALITY_BREAKDOWN); break;
        }
    } else if (direction == -1) {
        // Previous view
        _current_view_index = (_current_view_index == 0) ? 4 : _current_view_index - 1;
        switch (_current_view_index) {
            case 0: setState(STATE_WAVEFORM_LIVE); break;
            case 1: setState(STATE_POINTS_TABLE); break;
            case 2: setState(STATE_HARMONICS); break;
            case 3: setState(STATE_DECAY_ANALYSIS); break;
            case 4: setState(STATE_QUALITY_BREAKDOWN); break;
        }
    }
}

void AdvancedMeasurementUI::setState(AdvancedUIState state) {
    _state = state;
}

void AdvancedMeasurementUI::drawIdleScreen() {
    _display->clear();
    
    _display->setTextColor(COLOR_WHITE);
    _display->setTextSize(3);
    _display->setCursor(40, 20);
    _display->println("Coil Analyzer");
    
    _display->setTextSize(2);
    _display->setCursor(30, 80);
    _display->println("Advanced Analysis Mode");
    
    // Microphone info
    float dist, angle;
    _analyzer->getMicrophonePosition(dist, angle);
    drawMicrophoneStatus(20, 130);
    
    // Info box
    _display->setTextColor(COLOR_CYAN);
    _display->setTextSize(1);
    _display->setCursor(20, 180);
    _display->println("Multi-point Waveform Analysis");
    _display->setCursor(20, 195);
    _display->println("Measures 100 impact cycles");
    _display->setCursor(20, 210);
    _display->println("Analyzes: harmonics, decay, Q-factor");
    
    // START button
    _display->fillRect(150, 250, 180, 50, COLOR_GREEN);
    _display->drawRect(150, 250, 180, 50, COLOR_WHITE);
    _display->setTextColor(COLOR_BLACK, COLOR_GREEN);
    _display->setTextSize(2);
    _display->setCursor(180, 262);
    _display->println("START");
}

void AdvancedMeasurementUI::drawMeasuringScreen() {
    _display->clear();
    
    // Header
    _display->fillRect(0, 0, DISPLAY_WIDTH, 40, COLOR_GRAY);
    _display->setTextColor(COLOR_WHITE, COLOR_GRAY);
    _display->setTextSize(2);
    _display->setCursor(20, 8);
    _display->print("Measuring: ");
    _display->printf("%d%%", _analyzer->getMeasurementProgress());
    
    // Large CPS display
    drawCPSLarge(_analyzer->getCPS(), _analyzer->getQualityScore(), 50, 80);
    
    // Progress bar
    drawProgressBar(_analyzer->getMeasurementProgress(), 50, 160, 380, 20);
    
    // Impacts counter
    uint32_t impacts = _analyzer->getStats().samples_collected;
    _display->setTextColor(COLOR_CYAN);
    _display->setTextSize(2);
    _display->setCursor(50, 200);
    _display->printf("Impacts: %d / 100", impacts);
    
    // Last waveform preview (small)
    if (impacts > 0) {
        drawWaveformGraph(20, 240, 440, 60, false);
    }
}

void AdvancedMeasurementUI::drawWaveformLiveView() {
    _display->clear();
    
    // Title
    _display->fillRect(0, 0, DISPLAY_WIDTH, 30, COLOR_BLUE);
    _display->setTextColor(COLOR_WHITE, COLOR_BLUE);
    _display->setTextSize(2);
    _display->setCursor(20, 5);
    _display->println("Waveform Analysis");
    
    // Large waveform with points
    drawWaveformGraph(10, 40, 460, 200, true);
    
    // Statistics footer
    const auto& wf = _analyzer->getLastWaveform();
    _display->setTextColor(COLOR_YELLOW);
    _display->setTextSize(1);
    _display->setCursor(10, 250);
    _display->printf("Points: %d | Energy: %.2f | Peak: %.2f dB",
                     wf.point_count, wf.total_energy, wf.peak_amplitude_db);
    
    _display->setCursor(10, 265);
    _display->printf("Decay: %.3f | Q-Factor: %.2f | THD: %.1f%%",
                     wf.decay_rate, wf.quality_factor_q, wf.harmonic_content_percent);
    
    // Navigation hint
    _display->setTextColor(COLOR_LIGHT_GRAY);
    _display->setCursor(10, 300);
    _display->println("-> Next View  <- Prev View");
}

void AdvancedMeasurementUI::drawPointsTableView() {
    _display->clear();
    
    // Title
    _display->fillRect(0, 0, DISPLAY_WIDTH, 30, COLOR_BLUE);
    _display->setTextColor(COLOR_WHITE, COLOR_BLUE);
    _display->setTextSize(2);
    _display->setCursor(20, 5);
    _display->println("Measurement Points");
    
    // Draw table
    drawPointsTable(10, 40, 460, 260);
    
    // Navigation
    _display->setTextColor(COLOR_LIGHT_GRAY);
    _display->setTextSize(1);
    _display->setCursor(10, 300);
    _display->println("-> Next View  <- Prev View");
}

void AdvancedMeasurementUI::drawHarmonicsView() {
    _display->clear();
    
    // Title
    _display->fillRect(0, 0, DISPLAY_WIDTH, 30, COLOR_BLUE);
    _display->setTextColor(COLOR_WHITE, COLOR_BLUE);
    _display->setTextSize(2);
    _display->setCursor(20, 5);
    _display->println("Harmonic Analysis");
    
    // Draw harmonics chart
    drawHarmonicsChart(10, 40, 460, 250);
    
    // Navigation
    _display->setTextColor(COLOR_LIGHT_GRAY);
    _display->setTextSize(1);
    _display->setCursor(10, 300);
    _display->println("-> Next View  <- Prev View");
}

void AdvancedMeasurementUI::drawDecayAnalysisView() {
    _display->clear();
    
    // Title
    _display->fillRect(0, 0, DISPLAY_WIDTH, 30, COLOR_BLUE);
    _display->setTextColor(COLOR_WHITE, COLOR_BLUE);
    _display->setTextSize(2);
    _display->setCursor(20, 5);
    _display->println("Decay Analysis");
    
    // Draw decay curve
    drawDecayCurve(10, 40, 460, 220);
    
    // Statistics
    const auto& stats = _analyzer->getStats();
    _display->setTextColor(COLOR_YELLOW);
    _display->setTextSize(1);
    _display->setCursor(10, 270);
    _display->printf("Avg Decay Rate: %.4f", stats.mean_decay_rate);
    
    _display->setCursor(10, 285);
    _display->printf("Avg Q-Factor: %.2f", stats.mean_quality_factor);
    
    // Navigation
    _display->setTextColor(COLOR_LIGHT_GRAY);
    _display->setCursor(10, 300);
    _display->println("-> Next View  <- Prev View");
}

void AdvancedMeasurementUI::drawQualityBreakdownView() {
    _display->clear();
    
    // Title with overall score
    uint8_t quality = _analyzer->getQualityScore();
    uint16_t color = getQualityColorForValue(quality);
    
    _display->fillRect(0, 0, DISPLAY_WIDTH, 40, color);
    _display->setTextColor(COLOR_BLACK, color);
    _display->setTextSize(3);
    _display->setCursor(150, 5);
    _display->printf("%d%%", quality);
    
    _display->setTextColor(COLOR_WHITE);
    _display->setTextSize(2);
    _display->setCursor(20, 50);
    _display->println("Quality Breakdown");
    
    // Draw quality bars
    drawQualityBars(20, 90, 440, 180);
    
    // Navigation
    _display->setTextColor(COLOR_LIGHT_GRAY);
    _display->setTextSize(1);
    _display->setCursor(10, 300);
    _display->println("PRESS START to reset  <- Prev View");
}

void AdvancedMeasurementUI::drawErrorScreen(const char* error_msg) {
    _display->clear();
    _display->fillRect(0, 0, DISPLAY_WIDTH, 40, COLOR_RED);
    
    _display->setTextColor(COLOR_WHITE, COLOR_RED);
    _display->setTextSize(2);
    _display->setCursor(80, 8);
    _display->println("ERROR");
    
    _display->setTextColor(COLOR_WHITE);
    _display->setTextSize(2);
    _display->setCursor(20, 100);
    _display->println(error_msg);
    
    _display->setTextSize(1);
    _display->setCursor(20, 200);
    _display->println("Press START to retry");
}

void AdvancedMeasurementUI::drawWaveformGraph(int x, int y, int width, int height, bool with_points) {
    const auto& wf = _analyzer->getLastWaveform();
    if (wf.points.empty()) return;
    
    // Draw background
    _display->fillRect(x, y, width, height, COLOR_BLACK);
    _display->drawRect(x, y, width, height, COLOR_GRAY);
    
    // Draw grid
    drawGrid(x, y, width, height, 40);
    
    // Find min/max amplitude for scaling
    float max_amp = 0.1f;
    for (const auto& pt : wf.points) {
        max_amp = max(max_amp, pt.amplitude_linear);
    }
    
    // Draw waveform line connecting points
    for (size_t i = 0; i < wf.points.size() - 1; i++) {
        int x1 = x + (int)(wf.points[i].time_ms / 10.0f);
        int y1 = y + height / 2 - (int)(wf.points[i].amplitude_linear / max_amp * height / 2);
        
        int x2 = x + (int)(wf.points[i + 1].time_ms / 10.0f);
        int y2 = y + height / 2 - (int)(wf.points[i + 1].amplitude_linear / max_amp * height / 2);
        
        if (x1 < x + width && x2 < x + width) {
            _display->drawLine(x1, y1, x2, y2, COLOR_CYAN);
        }
    }
    
    // Draw measurement points
    if (with_points) {
        for (size_t i = 0; i < wf.points.size(); i++) {
            int px = x + (int)(wf.points[i].time_ms / 10.0f);
            int py = y + height / 2 - (int)(wf.points[i].amplitude_linear / max_amp * height / 2);
            
            uint16_t point_color = COLOR_WHITE;
            if (wf.points[i].type == AdvancedImpactAnalyzer::PEAK) point_color = COLOR_GREEN;
            else if (wf.points[i].type == AdvancedImpactAnalyzer::VALLEY) point_color = COLOR_RED;
            else if (wf.points[i].type == AdvancedImpactAnalyzer::RISING_EDGE) point_color = COLOR_YELLOW;
            
            drawPointMarker(px, py, i + 1, point_color);
        }
    }
}

void AdvancedMeasurementUI::drawPointsTable(int x, int y, int width, int height) {
    const auto& wf = _analyzer->getLastWaveform();
    if (wf.points.empty()) return;
    
    // Header
    _display->setTextColor(COLOR_YELLOW);
    _display->setTextSize(1);
    _display->setCursor(x, y);
    _display->println("#   Time    Amp    dB   Angle");
    
    // Points
    int row_height = 15;
    for (size_t i = 0; i < wf.points.size() && i < 15; i++) {
        const auto& pt = wf.points[i];
        
        uint16_t color = COLOR_WHITE;
        if (pt.type == AdvancedImpactAnalyzer::PEAK) color = COLOR_GREEN;
        else if (pt.type == AdvancedImpactAnalyzer::VALLEY) color = COLOR_RED;
        
        _display->setTextColor(color);
        _display->setCursor(x, y + (i + 1) * row_height);
        _display->printf("%2d %6.2f %5.3f %5.1f %6.1f°",
                         i + 1, pt.time_ms, pt.amplitude_linear,
                         pt.amplitude_db, pt.slope_angle_deg);
    }
}

void AdvancedMeasurementUI::drawHarmonicsChart(int x, int y, int width, int height) {
    const auto& stats = _analyzer->getStats();
    
    // Draw bars for harmonic ratios
    int bar_width = 60;
    int bar_x = x + 50;
    
    // Fundamental (Peak 1) - always 100%
    _display->fillRect(bar_x, y + height - 80, bar_width, 80, COLOR_GREEN);
    _display->drawRect(bar_x, y + height - 80, bar_width, 80, COLOR_WHITE);
    _display->setTextColor(COLOR_WHITE);
    _display->setTextSize(1);
    _display->setCursor(bar_x, y + height + 5);
    _display->println("Peak1");
    _display->setCursor(bar_x + 5, y + height - 85);
    _display->println("100%");
    
    // Peak 2 ratio
    int h2_height = (int)(stats.mean_harmonic_2_1 * 80);
    _display->fillRect(bar_x + 80, y + height - h2_height, bar_width, h2_height, COLOR_BLUE);
    _display->drawRect(bar_x + 80, y + height - h2_height, bar_width, h2_height, COLOR_WHITE);
    _display->setCursor(bar_x + 85, y + height + 5);
    _display->println("Peak2");
    _display->setCursor(bar_x + 85, y + height - h2_height - 10);
    _display->printf("%.0f%%", stats.mean_harmonic_2_1 * 100);
    
    // Peak 3 ratio
    int h3_height = (int)(stats.mean_harmonic_3_1 * 80);
    _display->fillRect(bar_x + 160, y + height - h3_height, bar_width, h3_height, COLOR_ORANGE);
    _display->drawRect(bar_x + 160, y + height - h3_height, bar_width, h3_height, COLOR_WHITE);
    _display->setCursor(bar_x + 165, y + height + 5);
    _display->println("Peak3");
    _display->setCursor(bar_x + 165, y + height - h3_height - 10);
    _display->printf("%.0f%%", stats.mean_harmonic_3_1 * 100);
    
    // Title
    _display->setTextColor(COLOR_CYAN);
    _display->setTextSize(1);
    _display->setCursor(x, y);
    _display->println("Harmonic Amplitude Ratios (ref: Peak1 = 100%)");
}

void AdvancedMeasurementUI::drawDecayCurve(int x, int y, int width, int height) {
    const auto& wf = _analyzer->getLastWaveform();
    
    // Draw background
    _display->fillRect(x, y, width, height, COLOR_BLACK);
    _display->drawRect(x, y, width, height, COLOR_GRAY);
    
    // Draw decay curve using peak amplitudes
    std::vector<const AdvancedImpactAnalyzer::MeasurementPoint*> peaks;
    for (const auto& pt : wf.points) {
        if (pt.type == AdvancedImpactAnalyzer::PEAK) {
            peaks.push_back(&pt);
        }
    }
    
    if (peaks.size() >= 2) {
        for (size_t i = 0; i < peaks.size() - 1; i++) {
            int x1 = x + (int)(peaks[i]->time_ms * width / 20.0f);
            int y1 = y + height - (int)(peaks[i]->amplitude_linear * height);
            
            int x2 = x + (int)(peaks[i + 1]->time_ms * width / 20.0f);
            int y2 = y + height - (int)(peaks[i + 1]->amplitude_linear * height);
            
            if (x1 < x + width && x2 < x + width) {
                _display->drawLine(x1, y1, x2, y2, COLOR_RED);
                _display->fillCircle(x1, y1, 3, COLOR_YELLOW);
            }
        }
    }
    
    // Stats
    _display->setTextColor(COLOR_YELLOW);
    _display->setTextSize(1);
    _display->setCursor(x + 5, y + 5);
    _display->printf("Decay: %.4f", wf.decay_rate);
}

void AdvancedMeasurementUI::drawQualityBars(int x, int y, int width, int height) {
    const auto& stats = _analyzer->getStats();
    
    // Consistency bar
    int consistency_width = (int)(stats.consistency_percent / 100.0f * (width - 20));
    _display->fillRect(x, y, consistency_width, 30, COLOR_GREEN);
    _display->drawRect(x, y, width - 20, 30, COLOR_GRAY);
    
    _display->setTextColor(COLOR_BLACK);
    _display->setTextSize(1);
    _display->setCursor(x + 5, y + 8);
    _display->printf("Consistency: %.1f%%", stats.consistency_percent);
    
    // Harmonic quality bar
    float harmonic_quality = (1.0f - abs(stats.mean_harmonic_2_1 - 0.5f)) * 100.0f;
    int harmonic_width = (int)(harmonic_quality / 100.0f * (width - 20));
    _display->fillRect(x, y + 50, harmonic_width, 30, COLOR_BLUE);
    _display->drawRect(x, y + 50, width - 20, 30, COLOR_GRAY);
    
    _display->setTextColor(COLOR_WHITE);
    _display->setCursor(x + 5, y + 58);
    _display->printf("Harmonic Quality: %.1f%%", harmonic_quality);
    
    // CPS stability
    float cps_stability = (1.0f - (stats.period_jitter_ms / stats.mean_period_ms)) * 100.0f;
    cps_stability = constrain(cps_stability, 0.0f, 100.0f);
    int cps_width = (int)(cps_stability / 100.0f * (width - 20));
    _display->fillRect(x, y + 100, cps_width, 30, COLOR_YELLOW);
    _display->drawRect(x, y + 100, width - 20, 30, COLOR_GRAY);
    
    _display->setTextColor(COLOR_BLACK);
    _display->setCursor(x + 5, y + 108);
    _display->printf("CPS Stability: %.1f%%", cps_stability);
}

void AdvancedMeasurementUI::drawMicrophoneStatus(int x, int y) {
    float distance, angle;
    _analyzer->getMicrophonePosition(distance, angle);
    
    _display->setTextColor(COLOR_LIGHT_GRAY);
    _display->setTextSize(1);
    _display->setCursor(x, y);
    _display->printf("Microphone: %.0f mm, %.0f°", distance, angle);
}

void AdvancedMeasurementUI::drawCPSLarge(float cps, uint8_t quality, int x, int y) {
    uint16_t color = getQualityColorForValue(quality);
    
    _display->setTextColor(color);
    _display->setTextSize(3);
    _display->setCursor(x, y);
    _display->printf("%.1f CPS", cps);
    
    _display->setTextColor(COLOR_WHITE);
    _display->setTextSize(2);
    _display->setCursor(x, y + 50);
    _display->printf("Quality: %d%%", quality);
}

void AdvancedMeasurementUI::drawProgressBar(uint8_t progress, int x, int y, int width, int height) {
    _display->drawRect(x, y, width, height, COLOR_GRAY);
    
    int fill_width = (progress * (width - 4)) / 100;
    uint16_t color = getQualityColorForValue(progress);
    
    _display->fillRect(x + 2, y + 2, fill_width, height - 4, color);
    
    _display->setTextColor(COLOR_WHITE);
    _display->setTextSize(1);
    _display->setCursor(x + width / 2 - 10, y + 3);
    _display->printf("%d%%", progress);
}

void AdvancedMeasurementUI::drawPointMarker(int px, int py, uint8_t point_number, uint16_t color) {
    _display->fillCircle(px, py, 4, color);
    _display->drawCircle(px, py, 4, COLOR_WHITE);
    
    _display->setTextColor(COLOR_BLACK, color);
    _display->setTextSize(1);
    _display->setCursor(px - 3, py - 2);
    _display->printf("%d", point_number);
}

void AdvancedMeasurementUI::drawGrid(int x, int y, int width, int height, int grid_spacing) {
    _display->setTextColor(COLOR_DARK_GRAY);
    
    // Vertical lines
    for (int xi = x; xi < x + width; xi += grid_spacing) {
        _display->drawLine(xi, y, xi, y + height, COLOR_DARK_GRAY);
    }
    
    // Horizontal lines
    for (int yi = y; yi < y + height; yi += grid_spacing) {
        _display->drawLine(x, yi, x + width, yi, COLOR_DARK_GRAY);
    }
}

uint16_t AdvancedMeasurementUI::getQualityColorForValue(uint8_t quality) const {
    if (quality >= 95) return COLOR_GREEN;
    if (quality >= 86) return COLOR_GREEN;
    if (quality >= 75) return COLOR_YELLOW;
    if (quality >= 61) return COLOR_ORANGE;
    return COLOR_RED;
}

int AdvancedMeasurementUI::mapValue(float val, float in_min, float in_max, int out_min, int out_max) const {
    return (int)((val - in_min) / (in_max - in_min) * (out_max - out_min) + out_min);
}

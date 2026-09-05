#ifndef ADVANCED_MEASUREMENT_UI_H
#define ADVANCED_MEASUREMENT_UI_H

#include <Arduino.h>
#include "display_manager.h"
#include "advanced_impact_analyzer.h"

/**
 * @class AdvancedMeasurementUI
 * @brief Advanced UI for multi-point waveform visualization and analysis
 * 
 * Features:
 * - Real-time waveform display with measurement points
 * - Point coordinate table (time, amplitude, angle)
 * - Harmonic analysis visualization
 * - Decay rate curve
 * - Spectral analysis detail
 * - Quality scoring with detailed breakdown
 * - Comparison mode (current vs reference)
 */
class AdvancedMeasurementUI {
public:
    enum AdvancedUIState {
        STATE_IDLE,
        STATE_MEASURING,
        STATE_WAVEFORM_LIVE,
        STATE_POINTS_TABLE,
        STATE_HARMONICS,
        STATE_DECAY_ANALYSIS,
        STATE_QUALITY_BREAKDOWN,
        STATE_COMPARISON,
        STATE_ERROR
    };
    
    AdvancedMeasurementUI();
    ~AdvancedMeasurementUI();
    
    /**
     * Initialize advanced UI
     * @param display Display manager instance
     * @param analyzer Advanced impact analyzer instance
     * @return true if successful
     */
    bool init(DisplayManager* display, AdvancedImpactAnalyzer* analyzer);
    
    /**
     * Update UI display (call regularly)
     */
    void update();
    
    /**
     * Handle START button press
     */
    void handleStartButton();
    
    /**
     * Handle navigation (left/right/up/down)
     */
    void handleNavigate(int8_t direction);  // -1=left, 1=right, -2=up, 2=down
    
    /**
     * Set UI state
     */
    void setState(AdvancedUIState state);
    
    /**
     * Get current UI state
     */
    AdvancedUIState getState() const { return _state; }
    
private:
    DisplayManager* _display;
    AdvancedImpactAnalyzer* _analyzer;
    AdvancedUIState _state;
    unsigned long _last_update_time;
    uint8_t _current_view_index;  // For multi-view navigation
    uint8_t _selected_point;      // For point detail view
    
    // Display methods for each state
    void drawIdleScreen();
    void drawMeasuringScreen();
    void drawWaveformLiveView();
    void drawPointsTableView();
    void drawHarmonicsView();
    void drawDecayAnalysisView();
    void drawQualityBreakdownView();
    void drawComparisonView();
    void drawErrorScreen(const char* error_msg);
    
    // Helper drawing methods
    void drawWaveformGraph(int x, int y, int width, int height, bool with_points = true);
    void drawPointsTable(int x, int y, int width, int height);
    void drawHarmonicsChart(int x, int y, int width, int height);
    void drawDecayCurve(int x, int y, int width, int height);
    void drawQualityBars(int x, int y, int width, int height);
    void drawMicrophoneStatus(int x, int y);
    void drawCPSLarge(float cps, uint8_t quality, int x, int y);
    void drawProgressBar(uint8_t progress, int x, int y, int width, int height);
    void drawPointMarker(int px, int py, uint8_t point_number, uint16_t color);
    void drawGrid(int x, int y, int width, int height, int grid_spacing);
    
    // Utility methods
    uint16_t getQualityColorForValue(uint8_t quality) const;
    int mapValue(float val, float in_min, float in_max, int out_min, int out_max) const;
};

#endif // ADVANCED_MEASUREMENT_UI_H

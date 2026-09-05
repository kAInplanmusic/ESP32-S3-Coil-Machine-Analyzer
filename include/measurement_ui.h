#ifndef MEASUREMENT_UI_H
#define MEASUREMENT_UI_H

#include <Arduino.h>
#include "config.h"
#include "display_manager.h"
#include "impact_analyzer.h"

/**
 * @class MeasurementUI
 * @brief User interface for measurement control and display
 * 
 * Features:
 * - START button to initiate 100-impact measurement
 * - Countdown timer showing remaining impacts
 * - Real-time CPS display with color coding
 * - Quality indicator (0-100%)
 * - Microphone position display
 */
class MeasurementUI {
public:
    enum MeasurementState {
        STATE_IDLE,           // Waiting for start
        STATE_MEASURING,      // Currently measuring 100 impacts
        STATE_COMPLETE,       // Measurement finished
        STATE_ERROR           // Error state
    };
    
    MeasurementUI();
    ~MeasurementUI();
    
    /**
     * Initialize UI
     * @param display Reference to display manager
     * @param analyzer Reference to impact analyzer
     */
    bool init(DisplayManager* display, ImpactAnalyzer* analyzer);
    
    /**
     * Update UI display
     */
    void update();
    
    /**
     * Handle START button press
     */
    void handleStartButton();
    
    /**
     * Draw idle screen (waiting for measurement)
     */
    void drawIdleScreen();
    
    /**
     * Draw measuring screen with countdown
     */
    void drawMeasuringScreen();
    
    /**
     * Draw results screen (measurement complete)
     */
    void drawResultsScreen();
    
    /**
     * Draw error screen
     * @param error_msg Error message to display
     */
    void drawErrorScreen(const char* error_msg);
    
    /**
     * Get current measurement state
     * @return Current state
     */
    MeasurementState getState() const { return _state; }
    
    /**
     * Set measurement state
     * @param state New state
     */
    void setState(MeasurementState state);
    
    /**
     * Get remaining impacts to measure
     * @return Count (0-100)
     */
    uint8_t getRemainingImpacts() const { return _remaining_impacts; }

private:
    DisplayManager* _display;
    ImpactAnalyzer* _analyzer;
    MeasurementState _state;
    uint8_t _remaining_impacts;
    unsigned long _last_update_time;
    
    /**
     * Draw large CPS value with color based on quality
     * @param cps CPS value
     * @param quality Quality percentage (0-100)
     * @param x X position
     * @param y Y position
     * @param huge_mode If true, use huge text for 95-100% quality
     */
    void drawCPSValue(float cps, uint8_t quality, int x, int y, bool huge_mode);
    
    /**
     * Draw countdown counter (100, 99, 98, ...)
     * @param remaining Remaining impacts (0-100)
     * @param x X position
     * @param y Y position
     */
    void drawCountdown(uint8_t remaining, int x, int y);
    
    /**
     * Draw START button
     * @param x X position
     * @param y Y position
     * @param active If true, highlight button
     */
    void drawStartButton(int x, int y, bool active);
    
    /**
     * Draw microphone position info
     * @param distance Distance in mm
     * @param angle Angle in degrees
     * @param x X position
     * @param y Y position
     */
    void drawMicrophoneInfo(float distance, float angle, int x, int y);
    
    /**
     * Draw statistics panel
     * @param stats Measurement statistics
     * @param x X position
     * @param y Y position
     */
    void drawStatsPanel(const ImpactAnalyzer::MeasurementStats& stats, int x, int y);
    
    /**
     * Get RGB565 color for quality level
     * @param quality Quality percentage (0-100)
     * @return RGB565 color code
     */
    uint16_t getQualityColorForValue(uint8_t quality) const;
};

#endif // MEASUREMENT_UI_H

#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "config.h"
#include "signal_processor.h"

/**
 * @class DisplayManager
 * @brief Manages TFT display rendering and UI
 * 
 * Features:
 * - Oscilloscope-style waveform display
 * - Real-time FFT spectrum visualization
 * - CPS and quality metrics display
 * - Touch interface support
 * - Efficient partial screen updates
 */
class DisplayManager {
public:
    enum ViewMode {
        VIEW_WAVEFORM,
        VIEW_SPECTRUM,
        VIEW_STATS,
        VIEW_COMBINED
    };
    
    DisplayManager();
    ~DisplayManager();
    
    /**
     * Initialize TFT display
     * @return true if successful
     */
    bool init();
    
    /**
     * Update display with current data
     * @param processor Signal processor with latest data
     * @param cps Current CPS value
     * @param qualityScore Quality score (0-100)
     */
    void update(const SignalProcessor& processor, float cps, uint8_t qualityScore);
    
    /**
     * Set view mode
     * @param mode Display mode to show
     */
    void setViewMode(ViewMode mode);
    
    /**
     * Get current view mode
     * @return Current view mode
     */
    ViewMode getViewMode() const { return _viewMode; }
    
    /**
     * Clear screen
     */
    void clear();
    
    /**
     * Set backlight brightness (0-255)
     * @param brightness Brightness level
     */
    void setBacklight(uint8_t brightness);
    
    /**
     * Draw initialization screen
     */
    void drawInitScreen();
    
    /**
     * Draw error message
     * @param title Error title
     * @param message Error message
     */
    void drawError(const char* title, const char* message);

private:
    TFT_eSPI _tft;
    ViewMode _viewMode;
    
    // Drawing buffers for optimization
    uint16_t* _waveformBuffer;
    uint16_t* _spectrumBuffer;
    
    // Screen state tracking for partial updates
    unsigned long _lastUpdateTime;
    
    // ========== Drawing Functions ==========
    
    /**
     * Draw combined view (waveform + spectrum + stats)
     */
    void drawCombinedView(const SignalProcessor& processor, 
                         float cps, uint8_t qualityScore);
    
    /**
     * Draw oscilloscope waveform
     * @param processor Signal processor
     * @param x X position
     * @param y Y position
     * @param width Display width
     * @param height Display height
     */
    void drawWaveform(const SignalProcessor& processor, 
                      int x, int y, int width, int height);
    
    /**
     * Draw FFT spectrum
     * @param spectrum Spectrum data
     * @param x X position
     * @param y Y position
     * @param width Display width
     * @param height Display height
     */
    void drawSpectrum(const SignalProcessor::SpectrumData& spectrum,
                      int x, int y, int width, int height);
    
    /**
     * Draw CPS gauge and value
     * @param cps Current CPS value
     * @param qualityScore Quality metric (0-100)
     * @param x X position
     * @param y Y position
     */
    void drawCPSGauge(float cps, uint8_t qualityScore, int x, int y);
    
    /**
     * Draw quality bar
     * @param score Quality score (0-100)
     * @param x X position
     * @param y Y position
     * @param width Width of bar
     * @param height Height of bar
     */
    void drawQualityBar(uint8_t score, int x, int y, int width, int height);
    
    /**
     * Draw statistics panel
     * @param processor Signal processor
     * @param stats Impact interval statistics
     * @param x X position
     * @param y Y position
     */
    void drawStatistics(const SignalProcessor& processor,
                       const std::vector<float>& stats, int x, int y);
    
    /**
     * Draw text with background
     * @param x X position
     * @param y Y position
     * @param text Text to draw
     * @param textColor Foreground color
     * @param bgColor Background color
     */
    void drawTextBox(int x, int y, const char* text, 
                    uint16_t textColor, uint16_t bgColor);
    
    /**
     * Format and draw numeric value
     * @param value Value to display
     * @param decimals Number of decimal places
     * @param unit Unit string
     * @param x X position
     * @param y Y position
     * @param fontSize Font size
     * @param color Text color
     */
    void drawNumber(float value, int decimals, const char* unit,
                   int x, int y, uint8_t fontSize, uint16_t color);
    
    /**
     * Draw horizontal grid lines for waveform
     * @param x X position
     * @param y Y position
     * @param width Display width
     * @param height Display height
     * @param divisions Number of grid divisions
     */
    void drawGrid(int x, int y, int width, int height, int divisions);
    
    /**
     * Draw frequency scale labels
     * @param x X position
     * @param y Y position
     * @param width Display width
     * @param freqMin Minimum frequency
     * @param freqMax Maximum frequency
     */
    void drawFrequencyScale(int x, int y, int width, 
                           float freqMin, float freqMax);
    
    /**
     * Get color for value based on scale
     * @param value Value (0-100)
     * @param minColor Color at 0%
     * @param maxColor Color at 100%
     * @return Interpolated color
     */
    uint16_t interpolateColor(float value, uint16_t minColor, uint16_t maxColor);
    
    /**
     * Draw menu/status bar
     * @param title Current view title
     * @param status Status text (e.g., "Recording", "Standby")
     */
    void drawStatusBar(const char* title, const char* status);
};

#endif // DISPLAY_MANAGER_H

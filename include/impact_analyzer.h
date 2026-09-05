#ifndef IMPACT_ANALYZER_H
#define IMPACT_ANALYZER_H

#include <Arduino.h>
#include <vector>
#include "config.h"

/**
 * @class ImpactAnalyzer
 * @brief Advanced 3-point impact analysis (A, B, C markers)
 * 
 * Measurement Points:
 * - C (Startpunkt): Trigger point, impulse beginning (t=0)
 * - A (Ausschlagpunkt): Peak amplitude, maximum excursion
 * - B (Wechselpunkt): Zero crossing or direction change
 * 
 * Analysis:
 * - Frequency range: 0-250 CPS
 * - Captures 100 impact cycles for statistical analysis
 * - Jitter measurement for consistency analysis
 * - Quality scoring based on timing stability
 */
class ImpactAnalyzer {
public:
    // Structure for single impact measurement
    struct ImpactMeasurement {
        unsigned long timestamp_ms;    // When impact was detected
        float time_C;                  // Start point (relative, ms)
        float time_A;                  // Peak point (relative, ms)
        float time_B;                  // Zero crossing (relative, ms)
        float amplitude_A;             // Peak amplitude (linear)
        float amplitude_A_db;          // Peak amplitude (dB)
        uint32_t cycle_count;          // Which cycle this is
    };
    
    // Statistics from 100-cycle measurement window
    struct MeasurementStats {
        float mean_period_ms;          // Average period (C to C) in ms
        float mean_time_A_ms;          // Average time to peak
        float mean_time_B_ms;          // Average time to zero crossing
        
        float jitter_A_ms;             // Jitter of point A (std dev)
        float jitter_B_ms;             // Jitter of point B (std dev)
        float jitter_C_ms;             // Jitter of point C (std dev)
        
        float cps_calculated;          // Calculated CPS from period
        float consistency_percent;     // 0-100% consistency score
        uint8_t quality_score;         // 0-100 quality rating
        
        uint32_t samples_collected;    // How many cycles measured
        bool measurement_complete;     // True when 100 cycles done
    };
    
    ImpactAnalyzer();
    ~ImpactAnalyzer();
    
    /**
     * Initialize impact analyzer
     * @return true if successful
     */
    bool init();
    
    /**
     * Set microphone position (for reference)
     * @param distance_mm Distance from source (mm)
     * @param angle_deg Angle from vertical (degrees)
     */
    void setMicrophonePosition(float distance_mm, float angle_deg);
    
    /**
     * Get current microphone position
     * @param distance Reference to store distance
     * @param angle Reference to store angle
     */
    void getMicrophonePosition(float& distance_mm, float& angle_deg) const;
    
    /**
     * Start measurement cycle (reset to begin measuring 100 impacts)
     */
    void startMeasurement();
    
    /**
     * Check if measurement is active
     * @return true if currently measuring
     */
    bool isMeasuring() const { return _measuring; }
    
    /**
     * Process audio sample and detect impacts
     * @param sample Audio sample (int16_t)
     * @param sample_index Current sample index in stream
     */
    void processSample(int16_t sample, uint32_t sample_index);
    
    /**
     * Detect impact event and extract timing points A, B, C
     * @param trigger_sample_idx Index of triggering sample (point C)
     * @param amplitude Peak amplitude detected
     * @return true if impact was recorded
     */
    bool recordImpact(uint32_t trigger_sample_idx, float amplitude);
    
    /**
     * Get current measurement statistics
     * @return Current stats structure
     */
    const MeasurementStats& getStats() const { return _stats; }
    
    /**
     * Get measurement progress (0-100%)
     * @return Percentage complete (0 if not measuring)
     */
    uint8_t getMeasurementProgress() const;
    
    /**
     * Get last recorded impact
     * @return Reference to last impact measurement
     */
    const ImpactMeasurement& getLastImpact() const { return _lastImpact; }
    
    /**
     * Get all recorded impacts in current cycle
     * @return Vector of impact measurements
     */
    const std::vector<ImpactMeasurement>& getImpacts() const { return _impacts; }
    
    /**
     * Stop measurement and compute final statistics
     * @return true if measurement was completed
     */
    bool stopMeasurement();
    
    /**
     * Get quality color for display
     * - Red: 0-60% (poor)
     * - Orange: 61-74% (fair)
     * - Yellow: 75-85% (good)
     * - Green: 86-94% (excellent)
     * - Bright Green: 95-100% (perfect)
     * @return Color code (RGB565)
     */
    uint16_t getQualityColor() const;
    
    /**
     * Reset all measurements
     */
    void reset();
    
    /**
     * Get CPS value (cycles per second)
     * Range: 0-250 CPS
     * @return Calculated CPS from current statistics
     */
    float getCPS() const { return _stats.cps_calculated; }
    
    /**
     * Get consistency percentage (0-100%)
     * Based on jitter of measurement points
     * @return Consistency score
     */
    float getConsistency() const { return _stats.consistency_percent; }

private:
    // Configuration
    float _microphone_distance_mm;
    float _microphone_angle_deg;
    
    // State
    bool _measuring;
    uint32_t _measurement_start_time;
    std::vector<ImpactMeasurement> _impacts;
    ImpactMeasurement _lastImpact;
    
    // Statistics
    MeasurementStats _stats;
    
    // Detection state
    int16_t _last_sample;
    bool _rising_edge_detected;
    uint32_t _edge_sample_idx;
    float _peak_amplitude;
    uint32_t _peak_sample_idx;
    
    // Peak detection buffer (for finding A and B points)
    static const size_t PEAK_BUFFER_SIZE = 2205;  // ~50ms @ 44.1kHz
    std::vector<int16_t> _peak_buffer;
    size_t _peak_buffer_pos;
    
    /**
     * Find peak (point A) in detected impulse
     * @param start_idx Start index in waveform
     * @return Index of peak
     */
    uint32_t findPeakPoint(uint32_t start_idx);
    
    /**
     * Find zero crossing (point B) after peak
     * @param peak_idx Index of peak (point A)
     * @return Index of zero crossing
     */
    uint32_t findZeroCrossing(uint32_t peak_idx);
    
    /**
     * Calculate period (C to C)
     * @return Period in milliseconds
     */
    float calculatePeriod();
    
    /**
     * Calculate jitter for each point
     */
    void calculateJitter();
    
    /**
     * Calculate consistency score (0-100%)
     * Based on combined jitter of A, B, C
     */
    void updateConsistencyScore();
    
    /**
     * Update quality score based on consistency
     */
    void updateQualityScore();
};

#endif // IMPACT_ANALYZER_H

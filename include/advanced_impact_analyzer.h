#ifndef ADVANCED_IMPACT_ANALYZER_H
#define ADVANCED_IMPACT_ANALYZER_H

#include <Arduino.h>
#include <vector>
#include <cmath>
#include "config.h"

/**
 * @class AdvancedImpactAnalyzer
 * @brief Multi-point waveform analysis with dynamic measurement matrix
 * 
 * Measurement Points (Variable Matrix):
 * 1. Baseline/Wendepunkt - Pre-impulse baseline detection
 * 2. Rising Edge (C-Point) - First impulse start
 * 3. Peak 1 (A-Point) - First maximum amplitude
 * 4. Valley 1 (B-Point) - First zero crossing/trough
 * 5. Peak 2 - Second harmonic peak
 * 6. Valley 2 - Second harmonic valley
 * 7. Peak 3 - Third harmonic peak
 * 8. Valley 3 - Third harmonic valley
 * ... up to 20+ points for full waveform characterization
 * 
 * Analysis Parameters:
 * - Time position (ms relative to start)
 * - Amplitude (linear + dB)
 * - Slope angle (rise/fall steepness in degrees)
 * - Energy per segment (area under curve)
 * - Harmonic ratios (Peak2/Peak1, Peak3/Peak1)
 * - Decay rate (exponential damping coefficient)
 * - Frequency estimation per segment
 */
class AdvancedImpactAnalyzer {
public:
    // Measurement point types
    enum PointType {
        BASELINE,           // Pre-impulse reference
        RISING_EDGE,        // Start of impulse (C-point)
        PEAK,              // Local maximum
        VALLEY,            // Local minimum
        ZERO_CROSSING,     // Zero crossing point
        END_IMPULSE        // Return to baseline
    };
    
    // Single measurement point in the waveform
    struct MeasurementPoint {
        uint8_t point_number;        // Sequential point ID (1-20+)
        PointType type;              // Type of this point
        
        float time_ms;               // Time from impulse start (ms)
        float amplitude_linear;      // Amplitude normalized 0-1
        float amplitude_db;          // Amplitude in dB
        
        int16_t raw_sample;          // Raw sample value at this point
        uint32_t sample_index;       // Index in audio stream
        
        // Angle Analysis
        float slope_angle_deg;       // Steepness of rise/fall (degrees)
        float rise_time_ms;          // Time from previous valley (ms)
        float fall_time_ms;          // Time to next valley (ms)
        
        // Energy & Frequency
        float segment_energy;        // Energy from prev point to this point
        float estimated_freq_hz;     // Estimated frequency at this point
        float phase_angle_deg;       // Phase angle (0-360°)
        
        // Consistency
        float jitter_ms;             // Jitter across 100 cycles (ms)
        float amplitude_std_dev;     // Standard deviation of amplitude
    };
    
    // Complete single impact measurement
    struct ImpactWaveform {
        uint32_t cycle_number;       // Which cycle this is (1-100)
        unsigned long timestamp_ms;  // When this impact occurred
        
        std::vector<MeasurementPoint> points;  // All measured points (dynamic)
        uint8_t point_count;         // Actual number of points detected
        
        // Overall Impact Characteristics
        float total_energy;          // Total energy of impact
        float peak_amplitude;        // Highest peak
        float peak_amplitude_db;     // Highest peak in dB
        uint8_t primary_peak_idx;    // Index of primary (first) peak
        
        // Harmonic Analysis
        float harmonic_ratio_2_1;    // Peak2 / Peak1 amplitude ratio
        float harmonic_ratio_3_1;    // Peak3 / Peak1 amplitude ratio
        float harmonic_content_percent;  // Total harmonic distortion
        
        // Damping & Decay
        float decay_rate;            // Exponential decay coefficient
        float quality_factor_q;      // Q-factor (oscillation quality)
        float envelope_area;         // Area under entire impulse envelope
        
        // Strike Quality
        uint8_t cleanliness_score;   // 0-100 (how clean the strike is)
        uint8_t consistency_score;   // 0-100 (compared to reference)
        uint8_t overall_quality;     // 0-100 (combined score)
    };
    
    // Statistics over 100 cycles
    struct AdvancedStats {
        uint32_t samples_collected;  // How many complete cycles measured
        bool measurement_complete;   // True when 100 cycles done
        
        // CPS and Timing
        float mean_period_ms;        // Average cycle period
        float cps_calculated;        // Calculated CPS (1000/period)
        float period_jitter_ms;      // Jitter of cycle period
        
        // Point-by-point statistics
        std::vector<float> point_time_means;      // Mean time for each point
        std::vector<float> point_time_std_devs;   // Std dev for each point
        std::vector<float> point_amplitude_means; // Mean amplitude for each point
        std::vector<float> point_amplitude_stds;  // Std dev for each point
        
        // Harmonic Statistics
        float mean_harmonic_2_1;     // Average Peak2/Peak1 ratio
        float mean_harmonic_3_1;     // Average Peak3/Peak1 ratio
        float harmonic_consistency;  // How consistent harmonics are (0-100%)
        
        // Decay Statistics
        float mean_decay_rate;       // Average decay coefficient
        float mean_quality_factor;   // Average Q-factor
        
        // Overall Quality
        float consistency_percent;   // 0-100% consistency across cycles
        uint8_t quality_score;       // 0-100 overall quality
        
        // Angle Statistics
        float mean_rise_slope;       // Average rise steepness (degrees)
        float mean_fall_slope;       // Average fall steepness (degrees)
    };
    
    AdvancedImpactAnalyzer();
    ~AdvancedImpactAnalyzer();
    
    /**
     * Initialize advanced analyzer
     * @param max_points Maximum measurement points to track (default 12)
     * @return true if successful
     */
    bool init(uint8_t max_points = 12);
    
    /**
     * Start new measurement cycle (100 impacts)
     */
    void startMeasurement();
    
    /**
     * Process audio sample for waveform analysis
     * @param sample Audio sample (int16_t)
     * @param sample_index Current sample index
     */
    void processSample(int16_t sample, uint32_t sample_index);
    
    /**
     * Detect and extract all measurement points from current impulse
     * @return true if valid impact detected with sufficient points
     */
    bool extractWaveformPoints();
    
    /**
     * Stop measurement and compute statistics
     * @return true if measurement complete (100+ cycles)
     */
    bool stopMeasurement();
    
    /**
     * Get current measurement statistics
     * @return Const reference to stats
     */
    const AdvancedStats& getStats() const { return _stats; }
    
    /**
     * Get all recorded waveforms
     * @return Vector of impact waveforms
     */
    const std::vector<ImpactWaveform>& getWaveforms() const { return _waveforms; }
    
    /**
     * Get last recorded waveform
     * @return Const reference to last impact
     */
    const ImpactWaveform& getLastWaveform() const { return _lastWaveform; }
    
    /**
     * Get measurement progress (0-100%)
     * @return Percentage complete
     */
    uint8_t getMeasurementProgress() const;
    
    /**
     * Check if currently measuring
     * @return true if active measurement
     */
    bool isMeasuring() const { return _measuring; }
    
    /**
     * Get CPS value
     * @return Current CPS (0-250)
     */
    float getCPS() const { return _stats.cps_calculated; }
    
    /**
     * Get consistency percentage
     * @return 0-100% consistency
     */
    float getConsistency() const { return _stats.consistency_percent; }
    
    /**
     * Get quality color for display
     * @return RGB565 color code
     */
    uint16_t getQualityColor() const;
    
    /**
     * Get quality score (0-100)
     * @return Quality percentage
     */
    uint8_t getQualityScore() const { return _stats.quality_score; }
    
    /**
     * Set microphone position
     * @param distance_mm Distance from coil (mm)
     * @param angle_deg Angle from vertical (degrees)
     */
    void setMicrophonePosition(float distance_mm, float angle_deg);
    
    /**
     * Get microphone position
     * @param distance Reference to store distance
     * @param angle Reference to store angle
     */
    void getMicrophonePosition(float& distance_mm, float& angle_deg) const;
    
    /**
     * Reset analyzer and clear all data
     */
    void reset();
    
private:
    // Configuration
    uint8_t _max_points;           // Maximum points to track per cycle
    float _microphone_distance_mm;
    float _microphone_angle_deg;
    
    // State
    bool _measuring;
    unsigned long _measurement_start_time;
    std::vector<ImpactWaveform> _waveforms;
    ImpactWaveform _lastWaveform;
    
    // Statistics
    AdvancedStats _stats;
    
    // Waveform buffer for current impulse
    static const size_t WAVEFORM_BUFFER_SIZE = 4410;  // ~100ms @ 44.1kHz
    std::vector<int16_t> _impulse_buffer;
    size_t _impulse_buffer_pos;
    bool _impulse_active;
    uint32_t _impulse_start_idx;
    
    // Sample tracking
    int16_t _last_sample;
    bool _rising_edge_detected;
    std::vector<int16_t> _segment_buffer;  // Current segment for energy calc
    
    /**
     * Find all peaks and valleys in impulse buffer
     * @return Vector of peak/valley indices
     */
    std::vector<uint32_t> findExtrema();
    
    /**
     * Calculate measurement point from sample index
     * @param sample_idx Index in impulse buffer
     * @return Populated measurement point
     */
    MeasurementPoint createMeasurementPoint(uint32_t sample_idx, PointType type);
    
    /**
     * Calculate slope angle between two points
     * @param idx1 First point index
     * @param idx2 Second point index
     * @return Angle in degrees
     */
    float calculateSlopeAngle(uint32_t idx1, uint32_t idx2);
    
    /**
     * Calculate energy (area under curve) between two points
     * @param idx1 Start point
     * @param idx2 End point
     * @return Energy value
     */
    float calculateSegmentEnergy(uint32_t idx1, uint32_t idx2);
    
    /**
     * Estimate local frequency at given point
     * @param point_idx Point index in buffer
     * @return Estimated frequency in Hz
     */
    float estimateLocalFrequency(uint32_t point_idx);
    
    /**
     * Calculate harmonic ratios (Peak2/Peak1, Peak3/Peak1)
     * @param waveform Waveform to analyze
     */
    void calculateHarmonics(ImpactWaveform& waveform);
    
    /**
     * Calculate decay rate (exponential damping)
     * @param waveform Waveform to analyze
     */
    void calculateDecayRate(ImpactWaveform& waveform);
    
    /**
     * Calculate overall impact quality score
     * @param waveform Waveform to score
     */
    void calculateQualityScore(ImpactWaveform& waveform);
    
    /**
     * Compute statistics from collected waveforms
     */
    void computeStatistics();
    
    /**
     * Update overall consistency and quality
     */
    void updateOverallQuality();
};

#endif // ADVANCED_IMPACT_ANALYZER_H

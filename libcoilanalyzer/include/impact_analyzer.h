#ifndef COILANALYZER_IMPACT_ANALYZER_H
#define COILANALYZER_IMPACT_ANALYZER_H

#include <vector>
#include <cstdint>
#include <cstddef>

namespace coilanalyzer {

/**
 * @class AdvancedImpactAnalyzer
 * @brief Platform-agnostic multi-point waveform analysis
 * 
 * Performs:
 * - Waveform extraction (up to 20 measurement points per cycle)
 * - Harmonic analysis (Peak ratios, THD calculation)
 * - Decay rate measurement (Exponential damping analysis)
 * - Quality scoring (Consistency, harmonic content, signal strength)
 */
class AdvancedImpactAnalyzer {
public:
    // Measurement point types
    enum PointType : uint8_t {
        BASELINE = 0,
        RISING_EDGE = 1,
        PEAK = 2,
        VALLEY = 3,
        ZERO_CROSSING = 4,
        END_IMPULSE = 5
    };
    
    // Single measurement point in the waveform
    struct MeasurementPoint {
        uint8_t point_number;              // Sequential point ID (1-20+)
        PointType type;                    // Type of this point
        
        float time_ms;                     // Time from impulse start (ms)
        float amplitude_linear;            // Amplitude normalized 0-1
        float amplitude_db;                // Amplitude in dB
        
        int16_t raw_sample;                // Raw sample value at this point
        uint32_t sample_index;             // Index in audio stream
        
        float slope_angle_deg;             // Steepness of rise/fall
        float rise_time_ms;                // Time from previous valley
        float fall_time_ms;                // Time to next valley
        
        float segment_energy;              // Energy from prev point to this
        float estimated_freq_hz;           // Estimated frequency at this point
        float phase_angle_deg;             // Phase angle (0-360°)
        
        float jitter_ms;                   // Jitter across cycles
        float amplitude_std_dev;           // Standard deviation of amplitude
    };
    
    // Complete single impact measurement
    struct ImpactWaveform {
        uint32_t cycle_number;             // Which cycle this is (1-100)
        unsigned long timestamp_ms;        // When this impact occurred
        
        std::vector<MeasurementPoint> points;  // All measured points
        uint8_t point_count;               // Actual number of points detected
        
        float total_energy;                // Total energy of impact
        float peak_amplitude;              // Highest peak
        float peak_amplitude_db;           // Highest peak in dB
        uint8_t primary_peak_idx;          // Index of primary peak
        
        float harmonic_ratio_2_1;          // Peak2 / Peak1 ratio
        float harmonic_ratio_3_1;          // Peak3 / Peak1 ratio
        float harmonic_content_percent;    // Total harmonic distortion
        
        float decay_rate;                  // Exponential decay coefficient
        float quality_factor_q;            // Q-factor (oscillation quality)
        float envelope_area;               // Area under entire impulse envelope
        
        uint8_t cleanliness_score;         // 0-100 (strike cleanliness)
        uint8_t consistency_score;         // 0-100 (consistency)
        uint8_t overall_quality;           // 0-100 (combined score)
    };
    
    // Statistics over multiple cycles
    struct AdvancedStats {
        uint32_t samples_collected;        // Number of complete cycles
        bool measurement_complete;         // True when target cycles done
        
        float mean_period_ms;              // Average cycle period
        float cps_calculated;              // Calculated CPS
        float period_jitter_ms;            // Jitter of cycle period
        
        std::vector<float> point_time_means;      // Mean time for each point
        std::vector<float> point_time_std_devs;   // Std dev for each point
        std::vector<float> point_amplitude_means; // Mean amplitude
        std::vector<float> point_amplitude_stds;  // Std dev of amplitude
        
        float mean_harmonic_2_1;           // Average Peak2/Peak1
        float mean_harmonic_3_1;           // Average Peak3/Peak1
        float thd_percent;                 // Total Harmonic Distortion %
        
        float mean_decay_rate;             // Average decay rate
        float mean_quality_factor;         // Average Q-factor
        
        uint8_t overall_quality_score;     // 0-100 combined quality
    };
    
    // Configuration
    struct Config {
        size_t max_points_per_waveform = 20;  // Max measurement points
        size_t max_harmonics = 5;             // Max harmonics to track
        size_t target_cycle_count = 100;      // Target measurements to collect
        float jitter_threshold_ms = 0.5f;     // Acceptable jitter
        size_t sample_rate = 44100;           // Sample rate in Hz
    };
    
    explicit AdvancedImpactAnalyzer(const Config& config = Config());
    ~AdvancedImpactAnalyzer();
    
    /**
     * Initialize the analyzer
     * @return true if successful
     */
    bool init();
    
    /**
     * Process a single audio sample
     * @param sample Raw audio sample
     * @param sample_index Position in audio stream
     */
    void process_sample(int16_t sample, uint32_t sample_index);
    
    /**
     * Start a new measurement session
     */
    void start_measurement();
    
    /**
     * Stop measurement and finalize statistics
     */
    void stop_measurement();
    
    /**
     * Check if currently measuring
     * @return true if measurement in progress
     */
    bool is_measuring() const { return is_measuring_; }
    
    /**
     * Get measurement progress (0-100%)
     * @return Progress percentage
     */
    uint8_t get_measurement_progress() const;
    
    /**
     * Get latest waveform if complete
     * @return Latest waveform or empty if not ready
     */
    const ImpactWaveform& get_latest_waveform() const {
        return latest_waveform_;
    }
    
    /**
     * Get accumulated statistics
     * @return Current statistics
     */
    const AdvancedStats& get_stats() const {
        return stats_;
    }
    
    /**
     * Get current CPS (cycles per second)
     * @return CPS value
     */
    float get_cps() const { return stats_.cps_calculated; }
    
    /**
     * Reset analyzer state
     */
    void reset();

private:
    Config config_;
    bool is_measuring_;
    
    std::vector<ImpactWaveform> collected_waveforms_;
    ImpactWaveform latest_waveform_;
    AdvancedStats stats_;
    
    std::vector<int16_t> current_buffer_;
    uint32_t measurement_start_index_;
    unsigned long measurement_start_time_ms_;
    
    // Internal helpers
    void analyze_waveform_();
    void calculate_statistics_();
    void detect_extrema_();
    void calculate_harmonics_();
    void calculate_decay_rate_();
    void calculate_quality_score_();
};

} // namespace coilanalyzer

#endif // COILANALYZER_IMPACT_ANALYZER_H

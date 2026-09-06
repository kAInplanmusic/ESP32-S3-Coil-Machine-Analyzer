#ifndef COILANALYZER_DATA_MODELS_H
#define COILANALYZER_DATA_MODELS_H

#include <string>
#include <vector>
#include <map>

namespace coilanalyzer {

/**
 * @class MeasurementResult
 * @brief Complete measurement result containing all analysis data
 * 
 * This is the primary data structure for serialization and export.
 */
class MeasurementResult {
public:
    // Metadata
    std::string device_id;
    std::string measurement_id;
    std::string timestamp;           // ISO 8601 format
    std::string software_version;
    
    // Configuration used
    size_t sample_rate;
    size_t fft_size;
    uint8_t measurement_point_count;
    uint32_t total_cycles_measured;
    
    // Results
    float average_cps;
    uint8_t overall_quality_score;   // 0-100
    
    // Waveform data (averaged/statistical)
    std::vector<float> avg_waveform_time;        // ms
    std::vector<float> avg_waveform_amplitude;   // normalized
    
    // Harmonic content
    std::vector<float> harmonic_frequencies;     // Hz
    std::vector<float> harmonic_amplitudes;      // relative to fundamental
    float thd_percent;                           // Total Harmonic Distortion
    
    // Quality metrics
    struct QualityMetrics {
        uint8_t consistency_score;               // 0-100
        uint8_t harmonic_purity_score;           // 0-100
        uint8_t signal_strength_score;           // 0-100
        uint8_t decay_regularity_score;          // 0-100
    } quality_metrics;
    
    // Decay analysis
    float decay_rate;
    float quality_factor_q;
    
    // Additional statistics
    struct Statistics {
        float period_mean_ms;
        float period_std_dev_ms;
        float peak_amplitude_mean;
        float peak_amplitude_std_dev;
        float signal_to_noise_ratio_db;
    } statistics;
    
    /**
     * Export to JSON string
     * @return JSON representation of measurement
     */
    std::string to_json() const;
    
    /**
     * Export to CSV format
     * @return CSV representation of measurement
     */
    std::string to_csv() const;
    
    /**
     * Export raw waveform data to CSV
     * @return CSV with time and amplitude columns
     */
    std::string export_waveform_csv() const;
    
    /**
     * Export harmonics to CSV
     * @return CSV with frequency and amplitude columns
     */
    std::string export_harmonics_csv() const;
};

/**
 * @class CalibrationData
 * @brief Microphone and system calibration data
 */
class CalibrationData {
public:
    float reference_level_db;        // Reference SPL level
    float microphone_gain_db;        // Current gain setting
    float noise_floor_db;            // Measured noise floor
    std::string calibration_date;    // ISO 8601 format
    
    /**
     * Load calibration from file
     * @param filepath Path to calibration file
     * @return true if successful
     */
    bool load(const std::string& filepath);
    
    /**
     * Save calibration to file
     * @param filepath Path to save calibration
     * @return true if successful
     */
    bool save(const std::string& filepath) const;
};

/**
 * @class AudioConfig
 * @brief Audio input configuration
 */
class AudioConfig {
public:
    size_t sample_rate = 44100;      // 44.1 kHz
    size_t bit_depth = 16;           // 16-bit
    size_t channels = 1;             // Mono
    size_t buffer_size = 4410;       // 100ms @ 44.1kHz
    
    // Gain settings
    float gain_db = 0.0f;
    float gain_min_db = -20.0f;
    float gain_max_db = 20.0f;
    
    /**
     * Validate configuration
     * @return true if valid
     */
    bool validate() const;
};

/**
 * @struct MeasurementSession
 * @brief Encapsulates a complete measurement session
 */
struct MeasurementSession {
    std::string session_id;
    std::string session_name;
    std::string created_timestamp;   // ISO 8601
    std::string completed_timestamp; // ISO 8601
    
    AudioConfig audio_config;
    MeasurementResult result;
    std::vector<std::string> notes;  // User-added notes
    
    bool is_complete = false;
    
    /**
     * Save session to file
     * @param filepath Directory to save session
     * @return true if successful
     */
    bool save(const std::string& filepath) const;
    
    /**
     * Load session from file
     * @param filepath Path to session file
     * @return true if successful
     */
    bool load(const std::string& filepath);
};

} // namespace coilanalyzer

#endif // COILANALYZER_DATA_MODELS_H

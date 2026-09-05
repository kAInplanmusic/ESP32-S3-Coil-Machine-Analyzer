#ifndef CALIBRATION_MANAGER_H
#define CALIBRATION_MANAGER_H

#include <Arduino.h>
#include "config.h"
#include "i2s_audio.h"

/**
 * @class CalibrationManager
 * @brief Advanced microphone calibration with noise profiling
 * 
 * Features:
 * - Automatic gain control calibration
 * - Noise floor measurement
 * - Reference waveform capture
 * - Frequency response profiling
 * - Environmental adaptation
 */
class CalibrationManager {
public:
    enum CalibrationState {
        STATE_IDLE,
        STATE_MEASURING_NOISE,
        STATE_CAPTURING_REFERENCE,
        STATE_ANALYZING,
        STATE_COMPLETE,
        STATE_ERROR
    };
    
    struct CalibrationData {
        float noise_floor_db;          // Minimum detectable signal
        float noise_std_dev;           // Noise variability
        float optimal_gain_db;         // Recommended gain setting
        float reference_peak_db;       // Peak from reference capture
        float frequency_response[32];  // Freq response bins (0-2kHz)
        uint32_t calibration_timestamp;
        bool is_valid;
    };
    
    CalibrationManager();
    ~CalibrationManager();
    
    /**
     * Initialize calibration manager
     * @param audio Audio input instance
     * @return true if successful
     */
    bool init(I2SAudio* audio);
    
    /**
     * Start calibration sequence
     * @param duration_ms How long to measure (5000ms typical)
     */
    void startCalibration(uint16_t duration_ms = 5000);
    
    /**
     * Update calibration process
     * @return true if calibration complete
     */
    bool update();
    
    /**
     * Get calibration state
     */
    CalibrationState getState() const { return _state; }
    
    /**
     * Get progress percentage (0-100)
     */
    uint8_t getProgress() const { return _progress; }
    
    /**
     * Get calibration data
     */
    const CalibrationData& getData() const { return _data; }
    
    /**
     * Apply calibration to audio input
     */
    void applyCalibration();
    
    /**
     * Save calibration to EEPROM
     */
    bool saveToEEPROM();
    
    /**
     * Load calibration from EEPROM
     */
    bool loadFromEEPROM();
    
    /**
     * Get calibration info string
     */
    String getInfoString() const;
    
private:
    I2SAudio* _audio;
    CalibrationState _state;
    CalibrationData _data;
    uint8_t _progress;
    unsigned long _start_time;
    uint16_t _duration_ms;
    
    static const size_t CALIB_BUFFER_SIZE = 4410;  // 100ms @ 44.1kHz
    int16_t _calib_buffer[CALIB_BUFFER_SIZE];
    size_t _calib_buffer_pos;
    
    /**
     * Measure noise floor
     */
    void measureNoiseFloor();
    
    /**
     * Capture reference waveform
     */
    void captureReference();
    
    /**
     * Analyze calibration results
     */
    void analyzeResults();
};

#endif // CALIBRATION_MANAGER_H

#include "calibration_manager.h"
#include <esp_log.h>
#include <EEPROM.h>
#include <cmath>

static const char* TAG = "CalibrationManager";
#define EEPROM_SIZE 512
#define CALIB_EEPROM_ADDR 0

CalibrationManager::CalibrationManager()
    : _audio(nullptr), _state(STATE_IDLE), _progress(0),
      _start_time(0), _duration_ms(5000), _calib_buffer_pos(0) {
    memset(&_data, 0, sizeof(_data));
    memset(_calib_buffer, 0, sizeof(_calib_buffer));
}

CalibrationManager::~CalibrationManager() {
}

bool CalibrationManager::init(I2SAudio* audio) {
    _audio = audio;
    ESP_LOGI(TAG, "Calibration Manager initialized");
    return true;
}

void CalibrationManager::startCalibration(uint16_t duration_ms) {
    _state = STATE_MEASURING_NOISE;
    _duration_ms = duration_ms;
    _start_time = millis();
    _progress = 0;
    _calib_buffer_pos = 0;
    
    ESP_LOGI(TAG, "Starting calibration sequence (%dms)...", duration_ms);
}

bool CalibrationManager::update() {
    if (_state == STATE_IDLE) {
        return true;
    }
    
    unsigned long elapsed = millis() - _start_time;
    _progress = (uint8_t)((elapsed * 100) / _duration_ms);
    _progress = constrain(_progress, 0, 100);
    
    switch (_state) {
        case STATE_MEASURING_NOISE:
            measureNoiseFloor();
            if (elapsed > _duration_ms / 3) {
                _state = STATE_CAPTURING_REFERENCE;
                _start_time = millis();
                _calib_buffer_pos = 0;
            }
            break;
            
        case STATE_CAPTURING_REFERENCE:
            captureReference();
            if (elapsed > _duration_ms / 3) {
                _state = STATE_ANALYZING;
                analyzeResults();
                _state = STATE_COMPLETE;
                ESP_LOGI(TAG, "Calibration complete!");
                ESP_LOGI(TAG, "  Noise floor: %.1f dB", _data.noise_floor_db);
                ESP_LOGI(TAG, "  Optimal gain: %.1f dB", _data.optimal_gain_db);
                ESP_LOGI(TAG, "  Reference peak: %.1f dB", _data.reference_peak_db);
                return true;
            }
            break;
            
        case STATE_COMPLETE:
            return true;
            
        case STATE_ERROR:
            ESP_LOGE(TAG, "Calibration error occurred");
            return false;
            
        default:
            break;
    }
    
    return false;
}

void CalibrationManager::measureNoiseFloor() {
    if (!_audio) return;
    
    // Capture samples
    int16_t temp_buffer[256];
    size_t samples_read = _audio->readSamples(temp_buffer, 256);
    
    if (samples_read > 0) {
        for (size_t i = 0; i < samples_read && _calib_buffer_pos < CALIB_BUFFER_SIZE; i++) {
            _calib_buffer[_calib_buffer_pos++] = temp_buffer[i];
        }
    }
    
    // Calculate noise statistics when buffer full
    if (_calib_buffer_pos >= CALIB_BUFFER_SIZE) {
        float sum = 0.0f, sum_sq = 0.0f;
        for (size_t i = 0; i < CALIB_BUFFER_SIZE; i++) {
            float sample = _calib_buffer[i] / 32767.0f;
            sum += abs(sample);
            sum_sq += sample * sample;
        }
        
        float mean = sum / CALIB_BUFFER_SIZE;
        float variance = (sum_sq / CALIB_BUFFER_SIZE) - (mean * mean);
        float rms = sqrt(variance);
        
        _data.noise_floor_db = 20.0f * log10(rms + 1e-6f);
        _data.noise_std_dev = sqrt(variance);
        
        _calib_buffer_pos = 0;  // Reset for next measurement
    }
}

void CalibrationManager::captureReference() {
    if (!_audio) return;
    
    // Capture reference samples (expecting a test signal/strike)
    int16_t temp_buffer[256];
    size_t samples_read = _audio->readSamples(temp_buffer, 256);
    
    if (samples_read > 0) {
        for (size_t i = 0; i < samples_read && _calib_buffer_pos < CALIB_BUFFER_SIZE; i++) {
            _calib_buffer[_calib_buffer_pos++] = temp_buffer[i];
        }
    }
}

void CalibrationManager::analyzeResults() {
    if (_calib_buffer_pos == 0) return;
    
    // Find peak in reference buffer
    int16_t peak_sample = 0;
    for (size_t i = 0; i < _calib_buffer_pos; i++) {
        if (abs(_calib_buffer[i]) > abs(peak_sample)) {
            peak_sample = _calib_buffer[i];
        }
    }
    
    float peak_linear = abs(peak_sample) / 32767.0f;
    _data.reference_peak_db = 20.0f * log10(peak_linear + 1e-6f);
    
    // Calculate optimal gain
    // Target: peak should be around -6dB (0.5 linear)
    float target_db = -6.0f;
    float current_db = _data.reference_peak_db;
    _data.optimal_gain_db = target_db - current_db;
    
    // Constrain gain to reasonable range
    _data.optimal_gain_db = constrain(_data.optimal_gain_db, -20.0f, 20.0f);
    
    _data.calibration_timestamp = millis();
    _data.is_valid = true;
}

void CalibrationManager::applyCalibration() {
    if (_audio && _data.is_valid) {
        float new_gain = _audio->getGain() + _data.optimal_gain_db;
        _audio->setGain(new_gain);
        ESP_LOGI(TAG, "Applied calibration gain: %.1f dB", new_gain);
    }
}

bool CalibrationManager::saveToEEPROM() {
    EEPROM.begin(EEPROM_SIZE);
    EEPROM.put(CALIB_EEPROM_ADDR, _data);
    bool result = EEPROM.commit();
    EEPROM.end();
    
    if (result) {
        ESP_LOGI(TAG, "Calibration saved to EEPROM");
    } else {
        ESP_LOGE(TAG, "Failed to save calibration");
    }
    
    return result;
}

bool CalibrationManager::loadFromEEPROM() {
    EEPROM.begin(EEPROM_SIZE);
    EEPROM.get(CALIB_EEPROM_ADDR, _data);
    EEPROM.end();
    
    if (_data.is_valid) {
        ESP_LOGI(TAG, "Calibration loaded from EEPROM");
        return true;
    } else {
        ESP_LOGE(TAG, "No valid calibration in EEPROM");
        return false;
    }
}

String CalibrationManager::getInfoString() const {
    char buffer[256];
    snprintf(buffer, sizeof(buffer),
             "Noise: %.1f dB | Gain: %.1f dB | Peak: %.1f dB",
             _data.noise_floor_db,
             _data.optimal_gain_db,
             _data.reference_peak_db);
    return String(buffer);
}

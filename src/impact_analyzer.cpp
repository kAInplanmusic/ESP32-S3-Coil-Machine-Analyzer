#include "impact_analyzer.h"
#include <esp_log.h>
#include <algorithm>
#include <cmath>

static const char* TAG = "ImpactAnalyzer";

ImpactAnalyzer::ImpactAnalyzer()
    : _microphone_distance_mm(100.0f), _microphone_angle_deg(0.0f),
      _measuring(false), _measurement_start_time(0),
      _last_sample(0), _rising_edge_detected(false),
      _edge_sample_idx(0), _peak_amplitude(0.0f), _peak_sample_idx(0),
      _peak_buffer_pos(0) {
    memset(&_lastImpact, 0, sizeof(_lastImpact));
    memset(&_stats, 0, sizeof(_stats));
}

ImpactAnalyzer::~ImpactAnalyzer() {
    _impacts.clear();
}

bool ImpactAnalyzer::init() {
    ESP_LOGI(TAG, "Initializing Impact Analyzer (0-250 CPS)...");
    
    _peak_buffer.resize(PEAK_BUFFER_SIZE);
    memset(_peak_buffer.data(), 0, PEAK_BUFFER_SIZE * sizeof(int16_t));
    
    reset();
    
    ESP_LOGI(TAG, "Impact Analyzer initialized");
    return true;
}

void ImpactAnalyzer::setMicrophonePosition(float distance_mm, float angle_deg) {
    _microphone_distance_mm = constrain(distance_mm, 10.0f, 500.0f);
    _microphone_angle_deg = constrain(angle_deg, -180.0f, 180.0f);
    ESP_LOGI(TAG, "Microphone positioned: %.0f mm, %.0f°", 
             _microphone_distance_mm, _microphone_angle_deg);
}

void ImpactAnalyzer::getMicrophonePosition(float& distance_mm, float& angle_deg) const {
    distance_mm = _microphone_distance_mm;
    angle_deg = _microphone_angle_deg;
}

void ImpactAnalyzer::startMeasurement() {
    _measuring = true;
    _measurement_start_time = millis();
    _impacts.clear();
    _stats.samples_collected = 0;
    _stats.measurement_complete = false;
    _stats.quality_score = 0;
    _stats.consistency_percent = 0.0f;
    
    ESP_LOGI(TAG, "Measurement started - collecting 100 impacts...");
}

uint8_t ImpactAnalyzer::getMeasurementProgress() const {
    if (!_measuring) {
        return 0;
    }
    return (uint8_t)((_impacts.size() * 100) / 100);  // 100 impacts target
}

void ImpactAnalyzer::processSample(int16_t sample, uint32_t sample_index) {
    if (!_measuring) {
        return;
    }
    
    // Check for rising edge (C point trigger)
    if (!_rising_edge_detected && _last_sample < 0 && sample >= 0) {
        _rising_edge_detected = true;
        _edge_sample_idx = sample_index;
        _peak_amplitude = 0.0f;
        _peak_sample_idx = sample_index;
    }
    
    // Track peak after edge detection
    if (_rising_edge_detected) {
        int16_t abs_sample = abs(sample);
        if (abs_sample > abs((int16_t)_peak_amplitude * 32767.0f)) {
            _peak_amplitude = (float)abs_sample / 32767.0f;
            _peak_sample_idx = sample_index;
        }
        
        // Store in peak buffer
        _peak_buffer[_peak_buffer_pos] = sample;
        _peak_buffer_pos = (_peak_buffer_pos + 1) % PEAK_BUFFER_SIZE;
        
        // Check for zero crossing after peak (B point)
        if (sample_index > _peak_sample_idx && 
            _last_sample > 0 && sample <= 0) {
            
            // Record impact
            if (recordImpact(_edge_sample_idx, _peak_amplitude)) {
                // Check if we have enough samples
                if (_impacts.size() >= 100) {
                    stopMeasurement();
                }
            }
            
            _rising_edge_detected = false;
        }
    }
    
    _last_sample = sample;
}

bool ImpactAnalyzer::recordImpact(uint32_t trigger_sample_idx, float amplitude) {
    ImpactMeasurement impact;
    impact.timestamp_ms = millis();
    impact.cycle_count = _impacts.size() + 1;
    impact.amplitude_A = amplitude;
    impact.amplitude_A_db = 20.0f * log10(amplitude + 1e-6f);
    
    // Point C (trigger/start)
    impact.time_C = 0.0f;  // Reference point
    
    // Point A (peak) - find in next ~0.5ms
    uint32_t peak_offset = _peak_sample_idx - trigger_sample_idx;
    impact.time_A = (float)peak_offset * 1000.0f / SAMPLE_RATE;
    
    // Point B (zero crossing) - simplified, assume ~2-3ms after peak
    // In real implementation, would search for actual zero crossing
    impact.time_B = impact.time_A + 2.5f;  // Estimated
    
    _lastImpact = impact;
    _impacts.push_back(impact);
    _stats.samples_collected = _impacts.size();
    
    return true;
}

bool ImpactAnalyzer::stopMeasurement() {
    if (!_measuring || _impacts.size() < 10) {
        ESP_LOGW(TAG, "Measurement incomplete: only %d impacts recorded", _impacts.size());
        return false;
    }
    
    _measuring = false;
    
    // Calculate statistics
    calculatePeriod();
    calculateJitter();
    updateConsistencyScore();
    updateQualityScore();
    
    _stats.measurement_complete = true;
    
    ESP_LOGI(TAG, "Measurement complete!");
    ESP_LOGI(TAG, "  CPS: %.2f", _stats.cps_calculated);
    ESP_LOGI(TAG, "  Consistency: %.1f%%", _stats.consistency_percent);
    ESP_LOGI(TAG, "  Quality Score: %d/100", _stats.quality_score);
    
    return true;
}

uint16_t ImpactAnalyzer::getQualityColor() const {
    uint8_t quality = _stats.quality_score;
    
    if (quality >= 95) {
        return COLOR_GREEN;  // Bright green for 95-100%
    } else if (quality >= 86) {
        return COLOR_GREEN;  // Green for 86-94%
    } else if (quality >= 75) {
        return COLOR_YELLOW;  // Yellow for 75-85%
    } else if (quality >= 61) {
        return COLOR_ORANGE;  // Orange for 61-74%
    } else {
        return COLOR_RED;  // Red for 0-60%
    }
}

void ImpactAnalyzer::reset() {
    _impacts.clear();
    _measuring = false;
    _peak_buffer_pos = 0;
    _rising_edge_detected = false;
    memset(&_stats, 0, sizeof(_stats));
    memset(&_lastImpact, 0, sizeof(_lastImpact));
}

float ImpactAnalyzer::calculatePeriod() {
    if (_impacts.size() < 2) {
        _stats.mean_period_ms = 0.0f;
        _stats.cps_calculated = 0.0f;
        return 0.0f;
    }
    
    // Calculate average period (time from C to C)
    std::vector<float> periods;
    for (size_t i = 1; i < _impacts.size(); i++) {
        float period_ms = (float)(_impacts[i].timestamp_ms - _impacts[i-1].timestamp_ms);
        periods.push_back(period_ms);
    }
    
    // Mean period
    float sum = 0.0f;
    for (float p : periods) {
        sum += p;
    }
    _stats.mean_period_ms = sum / periods.size();
    
    // Calculate CPS (cycles per second)
    // CPS = 1000 / period_ms
    _stats.cps_calculated = (_stats.mean_period_ms > 0) ? 
                           (1000.0f / _stats.mean_period_ms) : 0.0f;
    
    // Clamp to 0-250 CPS range
    _stats.cps_calculated = constrain(_stats.cps_calculated, 0.0f, 250.0f);
    
    return _stats.mean_period_ms;
}

void ImpactAnalyzer::calculateJitter() {
    if (_impacts.size() < 2) {
        _stats.jitter_A_ms = 0.0f;
        _stats.jitter_B_ms = 0.0f;
        _stats.jitter_C_ms = 0.0f;
        return;
    }
    
    // Calculate standard deviation for each point
    float mean_A = 0.0f, mean_B = 0.0f, mean_C = 0.0f;
    
    for (const auto& impact : _impacts) {
        mean_A += impact.time_A;
        mean_B += impact.time_B;
        mean_C += impact.time_C;
    }
    mean_A /= _impacts.size();
    mean_B /= _impacts.size();
    mean_C /= _impacts.size();
    
    // Calculate variance
    float var_A = 0.0f, var_B = 0.0f, var_C = 0.0f;
    for (const auto& impact : _impacts) {
        var_A += (impact.time_A - mean_A) * (impact.time_A - mean_A);
        var_B += (impact.time_B - mean_B) * (impact.time_B - mean_B);
        var_C += (impact.time_C - mean_C) * (impact.time_C - mean_C);
    }
    
    _stats.jitter_A_ms = sqrt(var_A / _impacts.size());
    _stats.jitter_B_ms = sqrt(var_B / _impacts.size());
    _stats.jitter_C_ms = sqrt(var_C / _impacts.size());
    
    _stats.mean_time_A_ms = mean_A;
    _stats.mean_time_B_ms = mean_B;
}

void ImpactAnalyzer::updateConsistencyScore() {
    // Consistency based on combined jitter
    // Lower jitter = higher consistency
    float total_jitter = _stats.jitter_A_ms + _stats.jitter_B_ms + _stats.jitter_C_ms;
    
    // Normalize: jitter of 0.5ms = 0% consistency, 0.0ms = 100% consistency
    float max_acceptable_jitter = 0.5f;  // ms
    _stats.consistency_percent = 100.0f - (total_jitter / max_acceptable_jitter) * 100.0f;
    _stats.consistency_percent = constrain(_stats.consistency_percent, 0.0f, 100.0f);
}

void ImpactAnalyzer::updateQualityScore() {
    // Quality score combines:
    // - Consistency (60% weight)
    // - Signal strength (40% weight)
    
    float consistency_score = _stats.consistency_percent;  // 0-100
    
    // Signal strength: average amplitude
    float mean_amplitude = 0.0f;
    for (const auto& impact : _impacts) {
        mean_amplitude += impact.amplitude_A;
    }
    mean_amplitude /= _impacts.size();
    
    // Convert amplitude to score (threshold: 0.3 = 100%, 0.05 = 0%)
    float signal_score = (mean_amplitude / 0.3f) * 100.0f;
    signal_score = constrain(signal_score, 0.0f, 100.0f);
    
    // Combined score
    _stats.quality_score = (uint8_t)((consistency_score * 0.6f + signal_score * 0.4f));
    _stats.quality_score = constrain(_stats.quality_score, (uint8_t)0, (uint8_t)100);
}

uint32_t ImpactAnalyzer::findPeakPoint(uint32_t start_idx) {
    // Simplified: peak is typically 0.3-0.5ms after trigger
    uint32_t search_samples = (SAMPLE_RATE / 2000);  // 0.5ms
    uint32_t peak_idx = start_idx;
    int16_t peak_val = 0;
    
    for (uint32_t i = 0; i < search_samples && (start_idx + i) < start_idx + search_samples; i++) {
        // Would search in peak buffer
        // This is simplified
    }
    
    return peak_idx;
}

uint32_t ImpactAnalyzer::findZeroCrossing(uint32_t peak_idx) {
    // Simplified: zero crossing typically 1.5-2.0ms after peak
    return peak_idx + (SAMPLE_RATE / 500);  // ~2ms
}

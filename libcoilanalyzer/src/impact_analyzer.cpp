#include "impact_analyzer.h"
#include <algorithm>
#include <cmath>
#include <numeric>

namespace coilanalyzer {

AdvancedImpactAnalyzer::AdvancedImpactAnalyzer(const Config& config)
    : config_(config), is_measuring_(false), measurement_start_index_(0), 
      measurement_start_time_ms_(0) {
}

AdvancedImpactAnalyzer::~AdvancedImpactAnalyzer() = default;

bool AdvancedImpactAnalyzer::init() {
    try {
        // Initialize waveform container
        latest_waveform_.points.resize(config_.max_points_per_waveform);
        
        // Initialize stats
        stats_.point_time_means.resize(config_.max_points_per_waveform, 0.0f);
        stats_.point_time_std_devs.resize(config_.max_points_per_waveform, 0.0f);
        stats_.point_amplitude_means.resize(config_.max_points_per_waveform, 0.0f);
        stats_.point_amplitude_stds.resize(config_.max_points_per_waveform, 0.0f);
        
        return true;
    } catch (...) {
        return false;
    }
}

void AdvancedImpactAnalyzer::process_sample(int16_t sample, uint32_t sample_index) {
    if (!is_measuring_) return;
    
    current_buffer_.push_back(sample);
    
    // Check if we have a complete waveform
    // This is a simplified detection - would be more sophisticated in production
    if (current_buffer_.size() > config_.sample_rate / 100) {  // 10ms buffer
        // Waveform analysis would happen here
    }
}

void AdvancedImpactAnalyzer::start_measurement() {
    is_measuring_ = true;
    measurement_start_index_ = 0;
    measurement_start_time_ms_ = 0;
    collected_waveforms_.clear();
    current_buffer_.clear();
}

void AdvancedImpactAnalyzer::stop_measurement() {
    is_measuring_ = false;
    calculate_statistics_();
}

uint8_t AdvancedImpactAnalyzer::get_measurement_progress() const {
    if (!is_measuring_) return 0;
    
    if (config_.target_cycle_count == 0) return 0;
    
    uint32_t progress = (stats_.samples_collected * 100) / config_.target_cycle_count;
    return static_cast<uint8_t>(std::min(progress, 100U));
}

void AdvancedImpactAnalyzer::reset() {
    is_measuring_ = false;
    collected_waveforms_.clear();
    current_buffer_.clear();
    stats_ = AdvancedStats();
    latest_waveform_ = ImpactWaveform();
}

void AdvancedImpactAnalyzer::analyze_waveform_() {
    if (current_buffer_.empty()) return;
    
    ImpactWaveform waveform;
    waveform.cycle_number = collected_waveforms_.size() + 1;
    waveform.timestamp_ms = static_cast<unsigned long>(
        (collected_waveforms_.size() * config_.sample_rate) / (config_.sample_rate / 1000)
    );
    
    // Detect extrema (peaks and valleys)
    detect_extrema_();
    
    // Calculate harmonics
    calculate_harmonics_();
    
    // Calculate decay rate
    calculate_decay_rate_();
    
    // Calculate quality score
    calculate_quality_score_();
    
    collected_waveforms_.push_back(waveform);
    latest_waveform_ = waveform;
    stats_.samples_collected = collected_waveforms_.size();
}

void AdvancedImpactAnalyzer::calculate_statistics_() {
    if (collected_waveforms_.empty()) {
        stats_.measurement_complete = false;
        return;
    }
    
    // Calculate mean period
    if (collected_waveforms_.size() > 1) {
        float total_time = 0.0f;
        for (size_t i = 1; i < collected_waveforms_.size(); ++i) {
            total_time += (collected_waveforms_[i].timestamp_ms - 
                          collected_waveforms_[i-1].timestamp_ms);
        }
        stats_.mean_period_ms = total_time / (collected_waveforms_.size() - 1);
        stats_.cps_calculated = 1000.0f / stats_.mean_period_ms;
    }
    
    // Calculate point statistics
    for (size_t point_idx = 0; point_idx < config_.max_points_per_waveform; ++point_idx) {
        std::vector<float> point_times;
        std::vector<float> point_amplitudes;
        
        for (const auto& waveform : collected_waveforms_) {
            if (point_idx < waveform.points.size()) {
                point_times.push_back(waveform.points[point_idx].time_ms);
                point_amplitudes.push_back(waveform.points[point_idx].amplitude_linear);
            }
        }
        
        if (!point_times.empty()) {
            // Calculate mean
            float time_sum = std::accumulate(point_times.begin(), point_times.end(), 0.0f);
            stats_.point_time_means[point_idx] = time_sum / point_times.size();
            
            // Calculate std dev
            float time_variance = 0.0f;
            for (float t : point_times) {
                time_variance += (t - stats_.point_time_means[point_idx]) * 
                                 (t - stats_.point_time_means[point_idx]);
            }
            stats_.point_time_std_devs[point_idx] = 
                std::sqrt(time_variance / point_times.size());
            
            // Similar for amplitude
            float amp_sum = std::accumulate(point_amplitudes.begin(), 
                                            point_amplitudes.end(), 0.0f);
            stats_.point_amplitude_means[point_idx] = amp_sum / point_amplitudes.size();
            
            float amp_variance = 0.0f;
            for (float a : point_amplitudes) {
                amp_variance += (a - stats_.point_amplitude_means[point_idx]) * 
                               (a - stats_.point_amplitude_means[point_idx]);
            }
            stats_.point_amplitude_stds[point_idx] = 
                std::sqrt(amp_variance / point_amplitudes.size());
        }
    }
    
    // Overall quality
    uint8_t consistency_score = 100 - static_cast<uint8_t>(
        std::min(100.0f, (stats_.period_jitter_ms / config_.jitter_threshold_ms) * 100.0f)
    );
    stats_.overall_quality_score = consistency_score;
    
    stats_.measurement_complete = 
        (stats_.samples_collected >= config_.target_cycle_count);
}

void AdvancedImpactAnalyzer::detect_extrema_() {
    // Find peaks and valleys in current buffer
    // This is simplified - production version would be more sophisticated
}

void AdvancedImpactAnalyzer::calculate_harmonics_() {
    // TODO: Implement harmonic calculation from FFT
}

void AdvancedImpactAnalyzer::calculate_decay_rate_() {
    // TODO: Implement exponential decay analysis
}

void AdvancedImpactAnalyzer::calculate_quality_score_() {
    // TODO: Implement quality scoring algorithm
}

} // namespace coilanalyzer

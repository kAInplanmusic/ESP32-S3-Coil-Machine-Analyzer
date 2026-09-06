#include "signal_processor.h"
#include <algorithm>
#include <cmath>
#include <numeric>

namespace coilanalyzer {

SignalProcessor::SignalProcessor(const Config& config)
    : config_(config), samples_accumulated_(0), last_peak_sample_(0) {
}

SignalProcessor::~SignalProcessor() = default;

bool SignalProcessor::init() {
    try {
        sample_buffer_.resize(config_.fft_size * 2);
        fft_input_.resize(config_.fft_size);
        fft_output_.resize(config_.fft_size);
        windowed_samples_.resize(config_.fft_size);
        
        // Initialize spectrum data
        spectrum_data_.magnitude.resize(config_.fft_size / 2);
        spectrum_data_.frequency.resize(config_.fft_size / 2);
        
        // Calculate frequency bins
        float freq_resolution = static_cast<float>(config_.sample_rate) / config_.fft_size;
        for (size_t i = 0; i < spectrum_data_.frequency.size(); ++i) {
            spectrum_data_.frequency[i] = i * freq_resolution;
        }
        
        return true;
    } catch (...) {
        return false;
    }
}

void SignalProcessor::process_samples(const int16_t* samples, size_t count) {
    if (!samples || count == 0) return;
    
    // Add samples to circular buffer
    for (size_t i = 0; i < count; ++i) {
        sample_buffer_[samples_accumulated_] = samples[i];
        samples_accumulated_++;
        
        if (samples_accumulated_ >= sample_buffer_.size()) {
            samples_accumulated_ = 0;
        }
    }
}

bool SignalProcessor::compute_fft() {
    if (samples_accumulated_ < config_.fft_size) {
        return false;
    }
    
    // Prepare FFT input with window
    apply_window();
    
    // TODO: Implement actual FFT computation
    // For now, this is a placeholder that would be replaced with kissfft or similar
    
    normalize_spectrum();
    detect_harmonics();
    
    return true;
}

bool SignalProcessor::detect_peaks(size_t min_distance, float threshold) {
    if (spectrum_data_.magnitude.empty()) {
        return false;
    }
    
    // Use provided threshold or use config default
    float actual_threshold = (threshold == 0.0f) ? config_.peak_threshold_db : threshold;
    
    // Find maximum
    float max_magnitude = *std::max_element(
        spectrum_data_.magnitude.begin(),
        spectrum_data_.magnitude.end()
    );
    
    // Detect peaks above threshold
    bool peaks_detected = false;
    ImpactEvent event;
    event.timestamp_us = last_peak_sample_ * 1000000 / config_.sample_rate;
    event.peak_amplitude_db = max_magnitude;
    event.peak_amplitude = std::pow(10.0f, max_magnitude / 20.0f);
    
    // Find fundamental frequency
    auto max_it = std::max_element(
        spectrum_data_.magnitude.begin(),
        spectrum_data_.magnitude.end()
    );
    
    if (max_it != spectrum_data_.magnitude.end()) {
        size_t peak_idx = std::distance(spectrum_data_.magnitude.begin(), max_it);
        event.fundamental_freq_hz = spectrum_data_.frequency[peak_idx];
        spectrum_data_.peak_frequency = event.fundamental_freq_hz;
        spectrum_data_.peak_magnitude = max_magnitude;
        
        impact_events_.push_back(event);
        peaks_detected = true;
    }
    
    last_peak_sample_ = samples_accumulated_;
    return peaks_detected;
}

size_t SignalProcessor::get_waveform(float* buffer, size_t samples) const {
    if (!buffer || samples == 0) return 0;
    
    size_t to_copy = std::min(samples, sample_buffer_.size());
    for (size_t i = 0; i < to_copy; ++i) {
        buffer[i] = sample_buffer_[i] / 32768.0f;  // Normalize to -1..1
    }
    return to_copy;
}

float SignalProcessor::calculate_cps() const {
    if (impact_events_.empty() || impact_events_.size() < 2) {
        return 0.0f;
    }
    
    // Calculate average period from peak intervals
    float total_interval_ms = 0.0f;
    for (size_t i = 1; i < impact_events_.size(); ++i) {
        unsigned long interval_us = impact_events_[i].timestamp_us - 
                                    impact_events_[i-1].timestamp_us;
        total_interval_ms += interval_us / 1000.0f;
    }
    
    float average_period_ms = total_interval_ms / (impact_events_.size() - 1);
    if (average_period_ms <= 0.0f) return 0.0f;
    
    return 1000.0f / average_period_ms;  // Convert period to CPS
}

float SignalProcessor::get_rms() const {
    if (sample_buffer_.empty()) return 0.0f;
    
    float sum_squares = 0.0f;
    for (int16_t sample : sample_buffer_) {
        float normalized = sample / 32768.0f;
        sum_squares += normalized * normalized;
    }
    
    return std::sqrt(sum_squares / sample_buffer_.size());
}

float SignalProcessor::get_peak_to_peak() const {
    if (sample_buffer_.empty()) return 0.0f;
    
    auto [min_it, max_it] = std::minmax_element(
        sample_buffer_.begin(),
        sample_buffer_.end()
    );
    
    float min_val = *min_it / 32768.0f;
    float max_val = *max_it / 32768.0f;
    
    return max_val - min_val;
}

void SignalProcessor::apply_window() {
    // Apply Hann window to samples
    for (size_t i = 0; i < config_.fft_size; ++i) {
        float window = 0.5f * (1.0f - std::cos(2.0f * 3.14159f * i / (config_.fft_size - 1)));
        windowed_samples_[i] = (sample_buffer_[i] / 32768.0f) * window;
    }
}

void SignalProcessor::normalize_spectrum() {
    // Normalize magnitude spectrum to dB
    for (size_t i = 0; i < spectrum_data_.magnitude.size(); ++i) {
        float mag = std::abs(fft_output_[i]);
        spectrum_data_.magnitude[i] = 20.0f * std::log10(mag + 1e-10f);
    }
}

void SignalProcessor::detect_harmonics() {
    // Find harmonics of fundamental frequency
    if (spectrum_data_.frequency.empty()) return;
    
    // TODO: Implement harmonic detection
    // This would find peaks at 2x, 3x, etc. the fundamental frequency
}

} // namespace coilanalyzer

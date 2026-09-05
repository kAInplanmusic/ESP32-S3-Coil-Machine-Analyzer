#ifndef COILANALYZER_SIGNAL_PROCESSOR_H
#define COILANALYZER_SIGNAL_PROCESSOR_H

#include <vector>
#include <cstddef>
#include <cstdint>

namespace coilanalyzer {

/**
 * @class SignalProcessor
 * @brief Platform-agnostic signal processing: FFT, peak detection, frequency analysis
 * 
 * This class is independent of any hardware or display layer.
 * Features:
 * - Real-time FFT computation
 * - Peak detection in frequency domain
 * - Bandpass filtering
 * - Harmonic analysis
 * - RMS and spectral statistics
 */
class SignalProcessor {
public:
    // Structure for impact event data
    struct ImpactEvent {
        unsigned long timestamp_us;    // Time of impact (microseconds)
        float peak_amplitude;          // Peak amplitude (linear, 0-1)
        float peak_amplitude_db;       // Peak amplitude (dB)
        float fundamental_freq_hz;     // Fundamental frequency (Hz)
        float total_energy;            // Total energy in signal
        float noise_floor_db;          // Noise floor level (dB)
        std::vector<float> harmonics;  // Harmonic frequencies and magnitudes
    };
    
    // Structure for spectrum data (for display)
    struct SpectrumData {
        std::vector<float> magnitude;     // FFT magnitude in dB
        std::vector<float> frequency;     // Frequency axis (Hz)
        float peak_magnitude;             // Peak magnitude (dB)
        float peak_frequency;             // Frequency at peak (Hz)
        float spectral_centroid;          // Weighted center of spectrum
    };
    
    // Configuration parameters
    struct Config {
        size_t fft_size = 2048;              // FFT size (must be power of 2)
        size_t sample_rate = 44100;          // Sample rate in Hz
        float peak_threshold_db = -20.0f;    // Minimum peak level (dB)
        float peak_min_distance_hz = 100.0f; // Min frequency distance between peaks
        size_t max_harmonics = 5;            // Maximum harmonics to detect
    };
    
    explicit SignalProcessor(const Config& config = Config());
    ~SignalProcessor();
    
    /**
     * Initialize signal processor
     * Allocates FFT buffers
     * @return true if successful
     */
    bool init();
    
    /**
     * Process audio samples and update internal state
     * @param samples Audio samples (int16_t, normalized -32768 to 32767)
     * @param count Number of samples
     */
    void process_samples(const int16_t* samples, size_t count);
    
    /**
     * Perform FFT on current buffer
     * @return true if FFT was computed
     */
    bool compute_fft();
    
    /**
     * Detect peaks in current signal
     * @param min_distance Minimum distance between peaks (samples)
     * @param threshold Threshold (dB) relative to maximum
     * @return true if new peaks were detected
     */
    bool detect_peaks(size_t min_distance = 0, float threshold = 0.0f);
    
    /**
     * Get detected impact events
     * @return Vector of impact events
     */
    const std::vector<ImpactEvent>& get_impact_events() const { 
        return impact_events_;
    }
    
    /**
     * Clear impact event history
     */
    void clear_impact_events() { 
        impact_events_.clear();
    }
    
    /**
     * Get spectrum data for display
     * @return Current spectrum data
     */
    const SpectrumData& get_spectrum_data() const { 
        return spectrum_data_;
    }
    
    /**
     * Get current time-domain waveform
     * @param buffer Output buffer
     * @param samples Number of samples to return
     * @return Number of samples returned
     */
    size_t get_waveform(float* buffer, size_t samples) const;
    
    /**
     * Calculate CPS (cycles per second) from current peaks
     * @return CPS value
     */
    float calculate_cps() const;
    
    /**
     * Get RMS value of current buffer
     * @return RMS value (linear)
     */
    float get_rms() const;
    
    /**
     * Get peak-to-peak value of current buffer
     * @return Peak-to-peak value
     */
    float get_peak_to_peak() const;

private:
    Config config_;
    std::vector<int16_t> sample_buffer_;
    std::vector<float> fft_input_;
    std::vector<float> fft_output_;
    std::vector<float> windowed_samples_;
    
    SpectrumData spectrum_data_;
    std::vector<ImpactEvent> impact_events_;
    
    size_t samples_accumulated_;
    unsigned long last_peak_sample_;
    
    // Internal helpers
    void apply_window();
    void normalize_spectrum();
    void detect_harmonics();
};

} // namespace coilanalyzer

#endif // COILANALYZER_SIGNAL_PROCESSOR_H

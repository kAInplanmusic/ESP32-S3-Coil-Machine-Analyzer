#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "coil_analyzer_core/CoreConfig.h"
#include "coil_analyzer_core/Logger.h"

namespace coil_analyzer_core {

class SignalProcessor {
public:
    struct ImpactEvent {
        std::uint64_t timestampMicros = 0;
        float peakAmplitude = 0.0f;
        float peakAmplitudeDb = 0.0f;
        float fundamentalFreq = 0.0f;
        float totalEnergy = 0.0f;
        float noiseFloor = 0.0f;
        std::vector<float> harmonics;
    };

    struct SpectrumData {
        std::vector<float> magnitudeDb;
        std::vector<float> frequencyHz;
        float peakMagnitudeDb = -80.0f;
        float peakFrequencyHz = 0.0f;
        float spectralCentroidHz = 0.0f;
    };

    explicit SignalProcessor(SignalProcessorConfig config = {}, Logger* logger = nullptr);

    void processSamples(const std::vector<std::int16_t>& samples,
                        std::uint64_t timestampMicros = 0);
    void processSamples(const std::int16_t* samples, std::size_t count,
                        std::uint64_t timestampMicros = 0);

    bool computeFft();
    bool detectPeaks(std::size_t minDistanceSamples = 0, float thresholdDb = 0.0f,
                     std::uint64_t timestampMicros = 0);

    const std::vector<ImpactEvent>& impactEvents() const noexcept;
    void clearImpactEvents();
    const SpectrumData& spectrumData() const noexcept;
    std::vector<std::int16_t> waveformData(std::size_t samples) const;
    float rmsEnergy() const noexcept;
    float peakToPeak() const noexcept;
    float noiseFloorDb() const noexcept;
    float calculateCps(float windowSec, std::uint64_t currentTimeMicros) const;
    std::vector<float> impactIntervalStats() const;
    void updateBandpassFilter(float freqLow, float freqHigh, float q = 1.0f);
    std::vector<std::int16_t> applyBandpassFilter(const std::vector<std::int16_t>& samples);
    std::vector<float> analyzeHarmonics(std::size_t harmonicCount = 5) const;
    std::uint8_t qualityScore() const noexcept;
    std::vector<std::size_t> findSpectrumPeaks(const std::vector<float>& magnitudesDb,
                                               float thresholdDb) const;
    float binIndexToFrequency(std::size_t binIndex) const;

private:
    struct FilterState {
        float x1 = 0.0f;
        float x2 = 0.0f;
        float y1 = 0.0f;
        float y2 = 0.0f;
        float b0 = 0.0f;
        float b1 = 0.0f;
        float b2 = 0.0f;
        float a1 = 0.0f;
        float a2 = 0.0f;
    };

    void initializeSpectrumAxes();
    void updateQualityScore();
    static float amplitudeToDb(float value);

    SignalProcessorConfig config_;
    Logger* logger_;
    NullLogger fallbackLogger_;
    std::vector<float> fftInput_;
    std::vector<float> fftReal_;
    std::vector<float> fftImag_;
    std::vector<float> window_;
    std::vector<std::int16_t> waveformBuffer_;
    std::size_t waveformWritePos_ = 0;
    SpectrumData spectrumData_;
    std::vector<ImpactEvent> impactEvents_;
    float rmsEnergy_ = 0.0f;
    float peakToPeak_ = 0.0f;
    float noiseFloorDb_ = -60.0f;
    FilterState filterState_;
    std::uint8_t qualityScore_ = 0;
};

}  // namespace coil_analyzer_core

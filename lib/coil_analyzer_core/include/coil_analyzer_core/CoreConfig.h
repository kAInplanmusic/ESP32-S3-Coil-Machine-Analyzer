#pragma once

#include <cstddef>
#include <cstdint>

namespace coil_analyzer_core {

struct CoreConfig {
    static constexpr std::size_t kDefaultSampleRate = 44100;
    static constexpr std::size_t kDefaultFftSize = 2048;
    static constexpr std::size_t kDefaultWaveformBufferSize = 4410;
    static constexpr std::size_t kDefaultQualityHistorySize = 100;
    static constexpr std::size_t kDefaultMaxWaveformPoints = 12;
    static constexpr float kDefaultPeakThresholdDb = -20.0f;
    static constexpr float kDefaultPeakMinDistanceMs = 5.0f;
    static constexpr float kDefaultFreqMinBand = 100.0f;
    static constexpr float kDefaultFreqMaxBand = 8000.0f;
    static constexpr float kDefaultJitterMaxAcceptableMs = 0.5f;
    static constexpr float kDefaultMinCps = 0.0f;
    static constexpr float kDefaultMaxCps = 250.0f;
};

struct SignalProcessorConfig {
    std::size_t sampleRate = CoreConfig::kDefaultSampleRate;
    std::size_t fftSize = CoreConfig::kDefaultFftSize;
    std::size_t waveformBufferSize = CoreConfig::kDefaultWaveformBufferSize;
    std::size_t qualityHistorySize = CoreConfig::kDefaultQualityHistorySize;
    float peakThresholdDb = CoreConfig::kDefaultPeakThresholdDb;
    float peakMinDistanceMs = CoreConfig::kDefaultPeakMinDistanceMs;
    float freqMinBand = CoreConfig::kDefaultFreqMinBand;
    float freqMaxBand = CoreConfig::kDefaultFreqMaxBand;
};

struct ImpactAnalyzerConfig {
    std::size_t sampleRate = CoreConfig::kDefaultSampleRate;
    std::uint8_t maxWaveformPoints = static_cast<std::uint8_t>(CoreConfig::kDefaultMaxWaveformPoints);
    std::size_t targetWaveformCount = 100;
    std::size_t waveformBufferSize = CoreConfig::kDefaultWaveformBufferSize;
    float baselineThreshold = 0.05f;
    float minImpulseSamples = static_cast<float>(CoreConfig::kDefaultSampleRate / 100);
    float jitterMaxAcceptableMs = CoreConfig::kDefaultJitterMaxAcceptableMs;
    float minCps = CoreConfig::kDefaultMinCps;
    float maxCps = CoreConfig::kDefaultMaxCps;
};

struct CalibrationConfig {
    std::size_t sampleRate = CoreConfig::kDefaultSampleRate;
    std::size_t bufferSize = CoreConfig::kDefaultWaveformBufferSize;
    float targetPeakDb = -6.0f;
    float minGainDb = -20.0f;
    float maxGainDb = 20.0f;
    std::size_t frequencyResponseBins = 32;
};

}  // namespace coil_analyzer_core

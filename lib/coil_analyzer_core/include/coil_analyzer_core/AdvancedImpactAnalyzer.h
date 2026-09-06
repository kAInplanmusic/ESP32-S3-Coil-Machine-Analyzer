#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "coil_analyzer_core/CoreConfig.h"
#include "coil_analyzer_core/Logger.h"

namespace coil_analyzer_core {

class AdvancedImpactAnalyzer {
public:
    enum class PointType : std::uint8_t {
        Baseline,
        RisingEdge,
        Peak,
        Valley,
        ZeroCrossing,
        EndImpulse,
    };

    struct MeasurementPoint {
        std::uint8_t pointNumber = 0;
        PointType type = PointType::Baseline;
        float timeMs = 0.0f;
        float amplitudeLinear = 0.0f;
        float amplitudeDb = 0.0f;
        std::int16_t rawSample = 0;
        std::uint32_t sampleIndex = 0;
        float slopeAngleDeg = 0.0f;
        float riseTimeMs = 0.0f;
        float fallTimeMs = 0.0f;
        float segmentEnergy = 0.0f;
        float estimatedFreqHz = 0.0f;
        float phaseAngleDeg = 0.0f;
        float jitterMs = 0.0f;
        float amplitudeStdDev = 0.0f;
    };

    struct ImpactWaveform {
        std::uint32_t cycleNumber = 0;
        std::uint64_t timestampMs = 0;
        std::vector<MeasurementPoint> points;
        std::uint8_t pointCount = 0;
        float totalEnergy = 0.0f;
        float peakAmplitude = 0.0f;
        float peakAmplitudeDb = 0.0f;
        std::uint8_t primaryPeakIdx = 0;
        float harmonicRatio2To1 = 0.0f;
        float harmonicRatio3To1 = 0.0f;
        float harmonicContentPercent = 0.0f;
        float decayRate = 0.0f;
        float qualityFactorQ = 0.0f;
        float envelopeArea = 0.0f;
        std::uint8_t cleanlinessScore = 0;
        std::uint8_t consistencyScore = 0;
        std::uint8_t overallQuality = 0;
    };

    struct AdvancedStats {
        std::uint32_t samplesCollected = 0;
        bool measurementComplete = false;
        float meanPeriodMs = 0.0f;
        float cpsCalculated = 0.0f;
        float periodJitterMs = 0.0f;
        std::vector<float> pointTimeMeans;
        std::vector<float> pointTimeStdDevs;
        std::vector<float> pointAmplitudeMeans;
        std::vector<float> pointAmplitudeStds;
        float meanHarmonic2To1 = 0.0f;
        float meanHarmonic3To1 = 0.0f;
        float harmonicConsistency = 0.0f;
        float meanDecayRate = 0.0f;
        float meanQualityFactor = 0.0f;
        float consistencyPercent = 0.0f;
        std::uint8_t qualityScore = 0;
        float meanRiseSlope = 0.0f;
        float meanFallSlope = 0.0f;
    };

    explicit AdvancedImpactAnalyzer(ImpactAnalyzerConfig config = {}, Logger* logger = nullptr);

    void startMeasurement();
    void processSample(std::int16_t sample, std::uint32_t sampleIndex, std::uint64_t timestampMs);
    bool extractWaveformPoints(std::uint64_t timestampMs);
    bool stopMeasurement();

    const AdvancedStats& stats() const noexcept;
    const std::vector<ImpactWaveform>& waveforms() const noexcept;
    const ImpactWaveform& lastWaveform() const noexcept;
    std::uint8_t measurementProgress() const noexcept;
    bool isMeasuring() const noexcept;
    float cps() const noexcept;
    float consistency() const noexcept;
    std::uint8_t qualityScore() const noexcept;
    void setMicrophonePosition(float distanceMm, float angleDeg);
    void getMicrophonePosition(float& distanceMm, float& angleDeg) const;
    void reset();
    std::vector<std::uint32_t> findExtrema() const;
    void calculateHarmonics(ImpactWaveform& waveform) const;
    void calculateDecayRate(ImpactWaveform& waveform) const;
    void calculateQualityScore(ImpactWaveform& waveform) const;
    void computeStatistics();
    void updateOverallQuality();

private:
    MeasurementPoint createMeasurementPoint(std::uint32_t sampleIdx, PointType type) const;
    float calculateSlopeAngle(std::uint32_t idx1, std::uint32_t idx2) const;
    float calculateSegmentEnergy(std::uint32_t idx1, std::uint32_t idx2) const;
    float estimateLocalFrequency(std::uint32_t pointIdx) const;
    static float amplitudeToDb(float value);

    ImpactAnalyzerConfig config_;
    Logger* logger_;
    NullLogger fallbackLogger_;
    float microphoneDistanceMm_ = 100.0f;
    float microphoneAngleDeg_ = 0.0f;
    bool measuring_ = false;
    std::uint64_t measurementStartTimeMs_ = 0;
    std::vector<ImpactWaveform> waveforms_;
    ImpactWaveform lastWaveform_;
    AdvancedStats stats_;
    std::vector<std::int16_t> impulseBuffer_;
    std::size_t impulseBufferPos_ = 0;
    bool impulseActive_ = false;
    std::uint32_t impulseStartIdx_ = 0;
    std::int16_t lastSample_ = 0;
    bool risingEdgeDetected_ = false;
    std::vector<std::int16_t> segmentBuffer_;
};

}  // namespace coil_analyzer_core

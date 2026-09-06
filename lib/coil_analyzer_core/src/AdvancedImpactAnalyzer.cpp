#include "coil_analyzer_core/AdvancedImpactAnalyzer.h"

#include <algorithm>
#include <cmath>
#include <numeric>

namespace coil_analyzer_core {
namespace {
constexpr float kPi = 3.14159265358979323846f;

template <typename T>
T clampValue(T value, T minValue, T maxValue) {
    return std::max(minValue, std::min(maxValue, value));
}
}  // namespace

AdvancedImpactAnalyzer::AdvancedImpactAnalyzer(ImpactAnalyzerConfig config, Logger* logger)
    : config_(config),
      logger_(logger != nullptr ? logger : &fallbackLogger_),
      impulseBuffer_(config_.waveformBufferSize, 0) {
    segmentBuffer_.reserve(config_.waveformBufferSize);
}

void AdvancedImpactAnalyzer::startMeasurement() {
    measuring_ = true;
    waveforms_.clear();
    stats_ = {};
}

void AdvancedImpactAnalyzer::processSample(std::int16_t sample, std::uint32_t sampleIndex,
                                           std::uint64_t timestampMs) {
    if (!measuring_) {
        return;
    }

    if (!risingEdgeDetected_ && lastSample_ < 0 && sample >= 0) {
        impulseActive_ = true;
        impulseStartIdx_ = sampleIndex;
        impulseBufferPos_ = 0;
        risingEdgeDetected_ = true;
        segmentBuffer_.clear();
    }

    if (impulseActive_ && impulseBufferPos_ < impulseBuffer_.size()) {
        impulseBuffer_[impulseBufferPos_++] = sample;
        segmentBuffer_.push_back(sample);
    }

    if (impulseActive_ && impulseBufferPos_ > static_cast<std::size_t>(config_.minImpulseSamples)) {
        const float currentLevel = std::abs(static_cast<float>(sample)) / 32767.0f;
        if (currentLevel < config_.baselineThreshold) {
            impulseActive_ = false;
            if (extractWaveformPoints(timestampMs) && waveforms_.size() >= config_.targetWaveformCount) {
                stopMeasurement();
            }
            risingEdgeDetected_ = false;
        }
    }

    lastSample_ = sample;
}

bool AdvancedImpactAnalyzer::extractWaveformPoints(std::uint64_t timestampMs) {
    if (segmentBuffer_.size() < 100) {
        return false;
    }

    ImpactWaveform waveform;
    waveform.cycleNumber = static_cast<std::uint32_t>(waveforms_.size() + 1);
    waveform.timestampMs = timestampMs;
    waveform.points.push_back(createMeasurementPoint(0, PointType::Baseline));
    waveform.points.push_back(createMeasurementPoint(0, PointType::RisingEdge));

    auto extrema = findExtrema();
    bool isPeak = !extrema.empty() && segmentBuffer_[extrema[0]] > 0;
    for (std::size_t i = 0; i < extrema.size() && waveform.points.size() < config_.maxWaveformPoints; ++i) {
        auto point = createMeasurementPoint(extrema[i], isPeak ? PointType::Peak : PointType::Valley);
        point.pointNumber = static_cast<std::uint8_t>(waveform.points.size() + 1);
        waveform.points.push_back(point);
        isPeak = !isPeak;
    }

    if (waveform.points.size() < config_.maxWaveformPoints) {
        auto endPoint = createMeasurementPoint(static_cast<std::uint32_t>(segmentBuffer_.size() - 1), PointType::EndImpulse);
        endPoint.pointNumber = static_cast<std::uint8_t>(waveform.points.size() + 1);
        waveform.points.push_back(endPoint);
    }

    waveform.pointCount = static_cast<std::uint8_t>(waveform.points.size());
    for (std::size_t i = 0; i < waveform.points.size(); ++i) {
        waveform.totalEnergy += waveform.points[i].segmentEnergy;
        if (waveform.points[i].type == PointType::Peak &&
            waveform.points[i].amplitudeLinear > waveform.peakAmplitude) {
            waveform.peakAmplitude = waveform.points[i].amplitudeLinear;
            waveform.peakAmplitudeDb = waveform.points[i].amplitudeDb;
            waveform.primaryPeakIdx = static_cast<std::uint8_t>(i);
        }
    }

    calculateHarmonics(waveform);
    calculateDecayRate(waveform);
    calculateQualityScore(waveform);

    lastWaveform_ = waveform;
    waveforms_.push_back(waveform);
    stats_.samplesCollected = static_cast<std::uint32_t>(waveforms_.size());
    return true;
}

bool AdvancedImpactAnalyzer::stopMeasurement() {
    if (!measuring_ || waveforms_.size() < 10) {
        return false;
    }
    measuring_ = false;
    computeStatistics();
    updateOverallQuality();
    stats_.measurementComplete = true;
    return true;
}

const AdvancedImpactAnalyzer::AdvancedStats& AdvancedImpactAnalyzer::stats() const noexcept { return stats_; }
const std::vector<AdvancedImpactAnalyzer::ImpactWaveform>& AdvancedImpactAnalyzer::waveforms() const noexcept { return waveforms_; }
const AdvancedImpactAnalyzer::ImpactWaveform& AdvancedImpactAnalyzer::lastWaveform() const noexcept { return lastWaveform_; }
bool AdvancedImpactAnalyzer::isMeasuring() const noexcept { return measuring_; }
float AdvancedImpactAnalyzer::cps() const noexcept { return stats_.cpsCalculated; }
float AdvancedImpactAnalyzer::consistency() const noexcept { return stats_.consistencyPercent; }
std::uint8_t AdvancedImpactAnalyzer::qualityScore() const noexcept { return stats_.qualityScore; }

std::uint8_t AdvancedImpactAnalyzer::measurementProgress() const noexcept {
    if (!measuring_ || config_.targetWaveformCount == 0) {
        return 0;
    }
    return static_cast<std::uint8_t>((waveforms_.size() * 100) / config_.targetWaveformCount);
}

void AdvancedImpactAnalyzer::setMicrophonePosition(float distanceMm, float angleDeg) {
    microphoneDistanceMm_ = clampValue(distanceMm, 10.0f, 500.0f);
    microphoneAngleDeg_ = clampValue(angleDeg, -180.0f, 180.0f);
}

void AdvancedImpactAnalyzer::getMicrophonePosition(float& distanceMm, float& angleDeg) const {
    distanceMm = microphoneDistanceMm_;
    angleDeg = microphoneAngleDeg_;
}

void AdvancedImpactAnalyzer::reset() {
    waveforms_.clear();
    measuring_ = false;
    impulseBufferPos_ = 0;
    impulseActive_ = false;
    risingEdgeDetected_ = false;
    stats_ = {};
    lastWaveform_ = {};
    segmentBuffer_.clear();
}

std::vector<std::uint32_t> AdvancedImpactAnalyzer::findExtrema() const {
    std::vector<std::uint32_t> extrema;
    if (segmentBuffer_.size() < 5) {
        return extrema;
    }
    for (std::size_t i = 2; i + 2 < segmentBuffer_.size(); ++i) {
        const auto current = segmentBuffer_[i];
        if (current > segmentBuffer_[i - 1] && current > segmentBuffer_[i - 2] &&
            current > segmentBuffer_[i + 1] && current > segmentBuffer_[i + 2]) {
            extrema.push_back(static_cast<std::uint32_t>(i));
        } else if (current < segmentBuffer_[i - 1] && current < segmentBuffer_[i - 2] &&
                   current < segmentBuffer_[i + 1] && current < segmentBuffer_[i + 2]) {
            extrema.push_back(static_cast<std::uint32_t>(i));
        }
    }
    return extrema;
}

AdvancedImpactAnalyzer::MeasurementPoint AdvancedImpactAnalyzer::createMeasurementPoint(
    std::uint32_t sampleIdx, PointType type) const {
    MeasurementPoint point;
    point.type = type;
    point.sampleIndex = sampleIdx;
    point.timeMs = static_cast<float>(sampleIdx) * 1000.0f / static_cast<float>(config_.sampleRate);
    if (sampleIdx < segmentBuffer_.size()) {
        point.rawSample = segmentBuffer_[sampleIdx];
        point.amplitudeLinear = std::abs(static_cast<float>(point.rawSample)) / 32767.0f;
        point.amplitudeDb = amplitudeToDb(point.amplitudeLinear);
    }
    if (sampleIdx > 1) {
        point.slopeAngleDeg = calculateSlopeAngle(sampleIdx - 1, sampleIdx);
    }
    if (sampleIdx > 0) {
        point.segmentEnergy = calculateSegmentEnergy(sampleIdx - 1, sampleIdx);
    }
    point.estimatedFreqHz = estimateLocalFrequency(sampleIdx);
    return point;
}

float AdvancedImpactAnalyzer::calculateSlopeAngle(std::uint32_t idx1, std::uint32_t idx2) const {
    if (idx1 >= segmentBuffer_.size() || idx2 >= segmentBuffer_.size()) {
        return 0.0f;
    }
    const float dy = static_cast<float>(segmentBuffer_[idx2] - segmentBuffer_[idx1]);
    const float angleRad = std::atan(dy / 32767.0f);
    return angleRad * 180.0f / kPi;
}

float AdvancedImpactAnalyzer::calculateSegmentEnergy(std::uint32_t idx1, std::uint32_t idx2) const {
    if (idx1 >= segmentBuffer_.size() || idx2 >= segmentBuffer_.size() || idx2 < idx1) {
        return 0.0f;
    }
    float energy = 0.0f;
    for (std::uint32_t i = idx1; i <= idx2; ++i) {
        energy += std::abs(static_cast<float>(segmentBuffer_[i])) / 32767.0f;
    }
    return energy / static_cast<float>(idx2 - idx1 + 1);
}

float AdvancedImpactAnalyzer::estimateLocalFrequency(std::uint32_t pointIdx) const {
    if (pointIdx < 10 || pointIdx + 10 >= segmentBuffer_.size()) {
        return 0.0f;
    }
    std::uint32_t zeroCrossings = 0;
    for (std::uint32_t i = pointIdx - 10; i < pointIdx + 10; ++i) {
        if (segmentBuffer_[i] * segmentBuffer_[i + 1] < 0) {
            ++zeroCrossings;
        }
    }
    if (zeroCrossings == 0) {
        return 0.0f;
    }
    const float periodSamples = 20.0f / static_cast<float>(zeroCrossings);
    return clampValue(static_cast<float>(config_.sampleRate) / periodSamples, 0.0f,
                      static_cast<float>(config_.sampleRate) / 2.0f);
}

void AdvancedImpactAnalyzer::calculateHarmonics(ImpactWaveform& waveform) const {
    float peak1 = 0.0f;
    float peak2 = 0.0f;
    float peak3 = 0.0f;
    std::uint8_t peakCount = 0;
    for (const auto& point : waveform.points) {
        if (point.type == PointType::Peak) {
            ++peakCount;
            if (peakCount == 1) peak1 = point.amplitudeLinear;
            else if (peakCount == 2) peak2 = point.amplitudeLinear;
            else if (peakCount == 3) peak3 = point.amplitudeLinear;
        }
    }
    if (peak1 > 0.01f) {
        waveform.harmonicRatio2To1 = peak2 / peak1;
        waveform.harmonicRatio3To1 = peak3 / peak1;
        waveform.harmonicContentPercent = ((peak2 + peak3) / peak1) * 100.0f;
    }
}

void AdvancedImpactAnalyzer::calculateDecayRate(ImpactWaveform& waveform) const {
    std::vector<const MeasurementPoint*> peaks;
    for (const auto& point : waveform.points) {
        if (point.type == PointType::Peak) {
            peaks.push_back(&point);
        }
        waveform.envelopeArea += point.segmentEnergy;
    }
    if (peaks.size() >= 2) {
        const float a1 = peaks[0]->amplitudeLinear;
        const float a2 = peaks[1]->amplitudeLinear;
        const float t1 = peaks[0]->timeMs;
        const float t2 = peaks[1]->timeMs;
        if (t2 > t1 && a1 > 0.01f && a2 > 0.0f) {
            waveform.decayRate = -std::log(a2 / a1) / (t2 - t1);
            if (waveform.decayRate > 0.0f) {
                waveform.qualityFactorQ = kPi / waveform.decayRate;
            }
        }
    }
}

void AdvancedImpactAnalyzer::calculateQualityScore(ImpactWaveform& waveform) const {
    int cleanliness = 100;
    if (waveform.harmonicContentPercent > 50.0f) {
        cleanliness -= static_cast<int>((waveform.harmonicContentPercent - 50.0f) / 2.0f);
    }
    waveform.cleanlinessScore = static_cast<std::uint8_t>(clampValue(cleanliness, 0, 100));

    if (!waveforms_.empty() && stats_.meanPeriodMs > 0.0f) {
        const float periodDiff = std::abs(static_cast<float>(waveform.timestampMs) - static_cast<float>(lastWaveform_.timestampMs));
        waveform.consistencyScore = static_cast<std::uint8_t>(clampValue(
            100.0f - std::abs(periodDiff - stats_.meanPeriodMs) / stats_.meanPeriodMs * 100.0f,
            0.0f, 100.0f));
    } else {
        waveform.consistencyScore = 100;
    }

    waveform.overallQuality = static_cast<std::uint8_t>((waveform.cleanlinessScore + waveform.consistencyScore) / 2);
}

void AdvancedImpactAnalyzer::computeStatistics() {
    if (waveforms_.size() < 2) {
        return;
    }

    stats_.pointTimeMeans.clear();
    stats_.pointTimeStdDevs.clear();
    stats_.pointAmplitudeMeans.clear();
    stats_.pointAmplitudeStds.clear();

    std::vector<float> periods;
    std::vector<float> decayRates;
    std::vector<float> qFactors;
    std::vector<float> riseSlopes;
    std::vector<float> fallSlopes;
    periods.reserve(waveforms_.size() - 1);

    for (std::size_t i = 1; i < waveforms_.size(); ++i) {
        periods.push_back(static_cast<float>(waveforms_[i].timestampMs - waveforms_[i - 1].timestampMs));
    }

    const float sumPeriods = std::accumulate(periods.begin(), periods.end(), 0.0f);
    stats_.meanPeriodMs = sumPeriods / static_cast<float>(periods.size());
    stats_.cpsCalculated = stats_.meanPeriodMs > 0.0f ? 1000.0f / stats_.meanPeriodMs : 0.0f;
    stats_.cpsCalculated = clampValue(stats_.cpsCalculated, config_.minCps, config_.maxCps);

    float periodVariance = 0.0f;
    for (float period : periods) {
        const float diff = period - stats_.meanPeriodMs;
        periodVariance += diff * diff;
    }
    stats_.periodJitterMs = std::sqrt(periodVariance / static_cast<float>(periods.size()));

    std::size_t maxPointCount = 0;
    for (const auto& waveform : waveforms_) {
        maxPointCount = std::max(maxPointCount, waveform.points.size());
        decayRates.push_back(waveform.decayRate);
        qFactors.push_back(waveform.qualityFactorQ);
        for (const auto& point : waveform.points) {
            if (point.type == PointType::Peak) riseSlopes.push_back(point.slopeAngleDeg);
            if (point.type == PointType::Valley) fallSlopes.push_back(point.slopeAngleDeg);
        }
    }

    for (std::size_t p = 0; p < maxPointCount; ++p) {
        std::vector<float> pointTimes;
        std::vector<float> pointAmplitudes;
        for (const auto& waveform : waveforms_) {
            if (p < waveform.points.size()) {
                pointTimes.push_back(waveform.points[p].timeMs);
                pointAmplitudes.push_back(waveform.points[p].amplitudeLinear);
            }
        }
        if (!pointTimes.empty()) {
            const float meanTime = std::accumulate(pointTimes.begin(), pointTimes.end(), 0.0f) / static_cast<float>(pointTimes.size());
            const float meanAmplitude = std::accumulate(pointAmplitudes.begin(), pointAmplitudes.end(), 0.0f) / static_cast<float>(pointAmplitudes.size());
            stats_.pointTimeMeans.push_back(meanTime);
            stats_.pointAmplitudeMeans.push_back(meanAmplitude);

            float timeVariance = 0.0f;
            float amplitudeVariance = 0.0f;
            for (std::size_t i = 0; i < pointTimes.size(); ++i) {
                timeVariance += (pointTimes[i] - meanTime) * (pointTimes[i] - meanTime);
                amplitudeVariance += (pointAmplitudes[i] - meanAmplitude) * (pointAmplitudes[i] - meanAmplitude);
            }
            stats_.pointTimeStdDevs.push_back(std::sqrt(timeVariance / static_cast<float>(pointTimes.size())));
            stats_.pointAmplitudeStds.push_back(std::sqrt(amplitudeVariance / static_cast<float>(pointAmplitudes.size())));
        }
    }

    for (const auto& waveform : waveforms_) {
        stats_.meanHarmonic2To1 += waveform.harmonicRatio2To1;
        stats_.meanHarmonic3To1 += waveform.harmonicRatio3To1;
    }
    stats_.meanHarmonic2To1 /= static_cast<float>(waveforms_.size());
    stats_.meanHarmonic3To1 /= static_cast<float>(waveforms_.size());
    stats_.harmonicConsistency = 100.0f - std::abs(stats_.meanHarmonic2To1 - 0.5f) * 100.0f;

    if (!decayRates.empty()) {
        stats_.meanDecayRate = std::accumulate(decayRates.begin(), decayRates.end(), 0.0f) / static_cast<float>(decayRates.size());
        stats_.meanQualityFactor = std::accumulate(qFactors.begin(), qFactors.end(), 0.0f) / static_cast<float>(qFactors.size());
    }
    if (!riseSlopes.empty()) {
        stats_.meanRiseSlope = std::accumulate(riseSlopes.begin(), riseSlopes.end(), 0.0f) / static_cast<float>(riseSlopes.size());
    }
    if (!fallSlopes.empty()) {
        stats_.meanFallSlope = std::accumulate(fallSlopes.begin(), fallSlopes.end(), 0.0f) / static_cast<float>(fallSlopes.size());
    }
}

void AdvancedImpactAnalyzer::updateOverallQuality() {
    if (!stats_.pointTimeStdDevs.empty()) {
        const float totalJitter = std::accumulate(stats_.pointTimeStdDevs.begin(), stats_.pointTimeStdDevs.end(), 0.0f);
        const float maxJitter = config_.jitterMaxAcceptableMs * static_cast<float>(stats_.pointTimeStdDevs.size());
        if (maxJitter > 0.0f) {
            stats_.consistencyPercent = clampValue(100.0f - (totalJitter / maxJitter) * 100.0f, 0.0f, 100.0f);
        }
    }
    const float harmonicQuality = (1.0f - std::abs(stats_.meanHarmonic2To1 - 0.5f)) * 100.0f;
    stats_.qualityScore = static_cast<std::uint8_t>(clampValue(stats_.consistencyPercent * 0.6f + harmonicQuality * 0.4f, 0.0f, 100.0f));
}

float AdvancedImpactAnalyzer::amplitudeToDb(float value) {
    return value > 0.0f ? 20.0f * std::log10(value) : -80.0f;
}

}  // namespace coil_analyzer_core

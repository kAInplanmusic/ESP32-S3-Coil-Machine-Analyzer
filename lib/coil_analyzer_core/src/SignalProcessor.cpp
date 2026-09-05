#include "coil_analyzer_core/SignalProcessor.h"

#include <algorithm>
#include <cmath>
#include <complex>
#include <numeric>
#include <sstream>

namespace coil_analyzer_core {
namespace {
constexpr float kPi = 3.14159265358979323846f;

float clampFloat(float value, float minValue, float maxValue) {
    return std::max(minValue, std::min(maxValue, value));
}
}  // namespace

SignalProcessor::SignalProcessor(SignalProcessorConfig config, Logger* logger)
    : config_(config),
      logger_(logger != nullptr ? logger : &fallbackLogger_),
      fftInput_(config_.fftSize, 0.0f),
      fftReal_(config_.fftSize / 2, 0.0f),
      fftImag_(config_.fftSize / 2, 0.0f),
      window_(config_.fftSize, 0.0f),
      waveformBuffer_(config_.waveformBufferSize, 0) {
    for (std::size_t i = 0; i < config_.fftSize; ++i) {
        const float n = static_cast<float>(i) / static_cast<float>(config_.fftSize - 1);
        window_[i] = 0.5f * (1.0f - std::cos(2.0f * kPi * n));
    }
    spectrumData_.magnitudeDb.assign(config_.fftSize / 2, -80.0f);
    spectrumData_.frequencyHz.assign(config_.fftSize / 2, 0.0f);
    initializeSpectrumAxes();
    updateBandpassFilter(config_.freqMinBand, config_.freqMaxBand);
}

void SignalProcessor::processSamples(const std::vector<std::int16_t>& samples,
                                     std::uint64_t timestampMicros) {
    processSamples(samples.data(), samples.size(), timestampMicros);
}

void SignalProcessor::processSamples(const std::int16_t* samples, std::size_t count,
                                     std::uint64_t) {
    if (samples == nullptr || count == 0) {
        return;
    }

    float sumSquared = 0.0f;
    std::int16_t minValue = samples[0];
    std::int16_t maxValue = samples[0];
    float noiseAccumulator = 0.0f;

    for (std::size_t i = 0; i < count; ++i) {
        waveformBuffer_[waveformWritePos_] = samples[i];
        waveformWritePos_ = (waveformWritePos_ + 1) % waveformBuffer_.size();

        const float normalized = static_cast<float>(samples[i]) / 32768.0f;
        sumSquared += normalized * normalized;
        noiseAccumulator += std::abs(normalized);
        minValue = std::min(minValue, samples[i]);
        maxValue = std::max(maxValue, samples[i]);
    }

    rmsEnergy_ = std::sqrt(sumSquared / static_cast<float>(count));
    peakToPeak_ = static_cast<float>(maxValue - minValue);
    noiseFloorDb_ = amplitudeToDb(noiseAccumulator / static_cast<float>(count));
}

bool SignalProcessor::computeFft() {
    if (config_.fftSize == 0 || waveformBuffer_.empty()) {
        return false;
    }

    const std::size_t fftBins = config_.fftSize / 2;
    for (std::size_t i = 0; i < config_.fftSize; ++i) {
        const std::size_t idx = (waveformWritePos_ + waveformBuffer_.size() - config_.fftSize + i) % waveformBuffer_.size();
        fftInput_[i] = (static_cast<float>(waveformBuffer_[idx]) / 32768.0f) * window_[i];
    }

    float peakMagDb = -80.0f;
    std::size_t peakIndex = 0;
    for (std::size_t k = 0; k < fftBins; ++k) {
        std::complex<float> sum(0.0f, 0.0f);
        for (std::size_t n = 0; n < config_.fftSize; ++n) {
            const float angle = -2.0f * kPi * static_cast<float>(k * n) / static_cast<float>(config_.fftSize);
            sum += std::polar(fftInput_[n], angle);
        }

        const float magnitude = (2.0f * std::abs(sum)) / static_cast<float>(config_.fftSize);
        const float magnitudeDb = amplitudeToDb(magnitude);
        spectrumData_.magnitudeDb[k] = magnitudeDb;
        fftReal_[k] = sum.real();
        fftImag_[k] = sum.imag();

        if (magnitudeDb > peakMagDb) {
            peakMagDb = magnitudeDb;
            peakIndex = k;
        }
    }

    spectrumData_.peakMagnitudeDb = peakMagDb;
    spectrumData_.peakFrequencyHz = binIndexToFrequency(peakIndex);

    float weightedFrequency = 0.0f;
    float weightSum = 0.0f;
    for (std::size_t k = 0; k < fftBins; ++k) {
        const float linear = std::pow(10.0f, spectrumData_.magnitudeDb[k] / 20.0f);
        weightedFrequency += spectrumData_.frequencyHz[k] * linear;
        weightSum += linear;
    }
    spectrumData_.spectralCentroidHz = weightSum > 0.0f ? weightedFrequency / weightSum : 0.0f;
    updateQualityScore();
    return true;
}

bool SignalProcessor::detectPeaks(std::size_t minDistanceSamples, float thresholdDb,
                                  std::uint64_t timestampMicros) {
    if (waveformBuffer_.size() < 3) {
        return false;
    }

    if (minDistanceSamples == 0) {
        minDistanceSamples = static_cast<std::size_t>(
            (config_.peakMinDistanceMs / 1000.0f) * static_cast<float>(config_.sampleRate));
    }
    if (thresholdDb == 0.0f) {
        thresholdDb = config_.peakThresholdDb;
    }

    std::int16_t maxSample = 0;
    std::size_t maxIdx = 0;
    for (std::size_t i = 1; i + 1 < waveformBuffer_.size(); ++i) {
        const auto prev = static_cast<std::int16_t>(std::abs(waveformBuffer_[i - 1]));
        const auto curr = static_cast<std::int16_t>(std::abs(waveformBuffer_[i]));
        const auto next = static_cast<std::int16_t>(std::abs(waveformBuffer_[i + 1]));
        if (curr > prev && curr > next && curr > maxSample) {
            maxSample = curr;
            maxIdx = i;
        }
    }

    const float thresholdLinear = std::pow(10.0f, thresholdDb / 20.0f);
    const bool farEnough = impactEvents_.empty() ||
        (timestampMicros - impactEvents_.back().timestampMicros) >
            static_cast<std::uint64_t>((1'000'000.0 * static_cast<double>(minDistanceSamples)) /
                                       static_cast<double>(config_.sampleRate));
    if (maxSample <= thresholdLinear * 32767.0f || !farEnough) {
        return false;
    }

    ImpactEvent event;
    event.timestampMicros = timestampMicros;
    event.peakAmplitude = static_cast<float>(maxSample) / 32767.0f;
    event.peakAmplitudeDb = amplitudeToDb(event.peakAmplitude);
    event.fundamentalFreq = spectrumData_.peakFrequencyHz;
    event.totalEnergy = rmsEnergy_;
    event.noiseFloor = noiseFloorDb_;
    event.harmonics = analyzeHarmonics();
    impactEvents_.push_back(event);

    if (impactEvents_.size() > config_.qualityHistorySize) {
        impactEvents_.erase(impactEvents_.begin());
    }

    updateQualityScore();
    (void)maxIdx;
    return true;
}

const std::vector<SignalProcessor::ImpactEvent>& SignalProcessor::impactEvents() const noexcept {
    return impactEvents_;
}

void SignalProcessor::clearImpactEvents() {
    impactEvents_.clear();
    qualityScore_ = 0;
}

const SignalProcessor::SpectrumData& SignalProcessor::spectrumData() const noexcept {
    return spectrumData_;
}

std::vector<std::int16_t> SignalProcessor::waveformData(std::size_t samples) const {
    samples = std::min(samples, waveformBuffer_.size());
    std::vector<std::int16_t> output(samples);
    const std::size_t start = (waveformWritePos_ + waveformBuffer_.size() - samples) % waveformBuffer_.size();
    for (std::size_t i = 0; i < samples; ++i) {
        output[i] = waveformBuffer_[(start + i) % waveformBuffer_.size()];
    }
    return output;
}

float SignalProcessor::rmsEnergy() const noexcept { return rmsEnergy_; }
float SignalProcessor::peakToPeak() const noexcept { return peakToPeak_; }
float SignalProcessor::noiseFloorDb() const noexcept { return noiseFloorDb_; }
std::uint8_t SignalProcessor::qualityScore() const noexcept { return qualityScore_; }

float SignalProcessor::calculateCps(float windowSec, std::uint64_t currentTimeMicros) const {
    if (impactEvents_.empty() || windowSec <= 0.0f) {
        return 0.0f;
    }
    const auto windowMicros = static_cast<std::uint64_t>(windowSec * 1'000'000.0f);
    const auto cutoff = currentTimeMicros > windowMicros ? currentTimeMicros - windowMicros : 0;
    std::size_t count = 0;
    for (const auto& event : impactEvents_) {
        if (event.timestampMicros >= cutoff) {
            ++count;
        }
    }
    return static_cast<float>(count) / windowSec;
}

std::vector<float> SignalProcessor::impactIntervalStats() const {
    std::vector<float> stats{0.0f, 0.0f, 0.0f, 0.0f};
    if (impactEvents_.size() < 2) {
        return stats;
    }

    std::vector<float> intervals;
    intervals.reserve(impactEvents_.size() - 1);
    for (std::size_t i = 1; i < impactEvents_.size(); ++i) {
        intervals.push_back(static_cast<float>(impactEvents_[i].timestampMicros - impactEvents_[i - 1].timestampMicros) / 1000.0f);
    }

    const float sum = std::accumulate(intervals.begin(), intervals.end(), 0.0f);
    stats[0] = sum / static_cast<float>(intervals.size());

    float variance = 0.0f;
    for (float interval : intervals) {
        const float diff = interval - stats[0];
        variance += diff * diff;
    }
    stats[1] = std::sqrt(variance / static_cast<float>(intervals.size()));
    stats[2] = *std::min_element(intervals.begin(), intervals.end());
    stats[3] = *std::max_element(intervals.begin(), intervals.end());
    return stats;
}

void SignalProcessor::updateBandpassFilter(float freqLow, float freqHigh, float q) {
    const float wc1 = 2.0f * kPi * freqLow / static_cast<float>(config_.sampleRate);
    const float wc2 = 2.0f * kPi * freqHigh / static_cast<float>(config_.sampleRate);
    const float wc = std::sqrt(wc1 * wc2);
    const float bw = wc2 - wc1;
    const float d = bw / std::max(q, 0.0001f);

    filterState_.a1 = -2.0f * std::cos(wc) / (1.0f + d);
    filterState_.a2 = (1.0f - d) / (1.0f + d);
    filterState_.b0 = d / (1.0f + d);
    filterState_.b1 = 0.0f;
    filterState_.b2 = -d / (1.0f + d);
}

std::vector<std::int16_t> SignalProcessor::applyBandpassFilter(const std::vector<std::int16_t>& samples) {
    std::vector<std::int16_t> output(samples.size(), 0);
    for (std::size_t i = 0; i < samples.size(); ++i) {
        const float x = static_cast<float>(samples[i]) / 32768.0f;
        const float y = filterState_.b0 * x + filterState_.b1 * filterState_.x1 +
                        filterState_.b2 * filterState_.x2 - filterState_.a1 * filterState_.y1 -
                        filterState_.a2 * filterState_.y2;
        filterState_.x2 = filterState_.x1;
        filterState_.x1 = x;
        filterState_.y2 = filterState_.y1;
        filterState_.y1 = y;
        output[i] = static_cast<std::int16_t>(clampFloat(y * 32767.0f, -32768.0f, 32767.0f));
    }
    return output;
}

std::vector<float> SignalProcessor::analyzeHarmonics(std::size_t harmonicCount) const {
    std::vector<float> harmonics;
    const float fundamental = spectrumData_.peakFrequencyHz;
    if (fundamental < 1.0f) {
        return harmonics;
    }

    harmonics.reserve(harmonicCount);
    for (std::size_t h = 1; h <= harmonicCount; ++h) {
        const std::size_t bin = static_cast<std::size_t>((fundamental * static_cast<float>(h) * static_cast<float>(config_.fftSize)) /
                                                         static_cast<float>(config_.sampleRate));
        if (bin < spectrumData_.magnitudeDb.size()) {
            harmonics.push_back(spectrumData_.magnitudeDb[bin]);
        }
    }
    return harmonics;
}

std::vector<std::size_t> SignalProcessor::findSpectrumPeaks(const std::vector<float>& magnitudesDb,
                                                            float thresholdDb) const {
    std::vector<std::size_t> peaks;
    if (magnitudesDb.size() < 3) {
        return peaks;
    }
    for (std::size_t i = 1; i + 1 < magnitudesDb.size(); ++i) {
        if (magnitudesDb[i] > magnitudesDb[i - 1] && magnitudesDb[i] > magnitudesDb[i + 1] &&
            magnitudesDb[i] > thresholdDb) {
            peaks.push_back(i);
        }
    }
    return peaks;
}

float SignalProcessor::binIndexToFrequency(std::size_t binIndex) const {
    return static_cast<float>(binIndex) * static_cast<float>(config_.sampleRate) /
           static_cast<float>(config_.fftSize);
}

void SignalProcessor::initializeSpectrumAxes() {
    for (std::size_t i = 0; i < spectrumData_.frequencyHz.size(); ++i) {
        spectrumData_.frequencyHz[i] = binIndexToFrequency(i);
    }
}

void SignalProcessor::updateQualityScore() {
    const auto stats = impactIntervalStats();
    if (stats[0] < 0.1f) {
        qualityScore_ = spectrumData_.peakMagnitudeDb > -10.0f ? 50 : 0;
        return;
    }

    const float cv = (stats[1] / stats[0]) * 100.0f;
    const float consistencyScore = 100.0f - std::min(cv, 50.0f);
    float spectralScore = 50.0f;
    if (spectrumData_.peakMagnitudeDb > -10.0f) {
        spectralScore = 100.0f;
    } else if (spectrumData_.peakMagnitudeDb > -30.0f) {
        spectralScore = 75.0f;
    } else if (spectrumData_.peakMagnitudeDb > -50.0f) {
        spectralScore = 50.0f;
    }
    qualityScore_ = static_cast<std::uint8_t>(std::min(100.0f, consistencyScore * 0.5f + spectralScore * 0.5f));
}

float SignalProcessor::amplitudeToDb(float value) {
    return value > 0.0f ? 20.0f * std::log10(value) : -80.0f;
}

}  // namespace coil_analyzer_core

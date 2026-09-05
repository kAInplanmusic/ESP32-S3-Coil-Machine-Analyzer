#include "coil_analyzer_core/CalibrationManager.h"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <sstream>

namespace coil_analyzer_core {
namespace {
float clampFloat(float value, float minValue, float maxValue) {
    return std::max(minValue, std::min(maxValue, value));
}
}  // namespace

CalibrationManager::CalibrationManager(CalibrationConfig config, Logger* logger)
    : config_(config),
      logger_(logger != nullptr ? logger : &fallbackLogger_),
      calibrationBuffer_(config_.bufferSize, 0) {
    data_.frequencyResponse.assign(config_.frequencyResponseBins, 0.0f);
}

void CalibrationManager::attachAudioInput(AudioInput* audioInput) {
    audioInput_ = audioInput;
}

void CalibrationManager::startCalibration(std::uint32_t durationMs) {
    reset();
    state_ = CalibrationState::MeasuringNoise;
    durationMs_ = durationMs;
}

bool CalibrationManager::update(std::uint64_t currentTimeMs) {
    if (state_ == CalibrationState::Idle) {
        return true;
    }
    if (startTimeMs_ == 0) {
        startTimeMs_ = currentTimeMs;
    }

    const auto elapsed = currentTimeMs - startTimeMs_;
    progress_ = static_cast<std::uint8_t>(std::min<std::uint64_t>(100, (elapsed * 100) / std::max<std::uint32_t>(1, durationMs_)));

    switch (state_) {
        case CalibrationState::MeasuringNoise:
            measureNoiseFloor();
            if (elapsed > durationMs_ / 3) {
                state_ = CalibrationState::CapturingReference;
                startTimeMs_ = currentTimeMs;
                calibrationBufferPos_ = 0;
            }
            break;
        case CalibrationState::CapturingReference:
            captureReference();
            if (elapsed > durationMs_ / 3) {
                state_ = CalibrationState::Analyzing;
                analyzeResults(currentTimeMs);
                state_ = CalibrationState::Complete;
                return true;
            }
            break;
        case CalibrationState::Complete:
            return true;
        case CalibrationState::Error:
            return false;
        case CalibrationState::Analyzing:
        case CalibrationState::Idle:
            break;
    }
    return false;
}

CalibrationManager::CalibrationState CalibrationManager::state() const noexcept { return state_; }
std::uint8_t CalibrationManager::progress() const noexcept { return progress_; }
const CalibrationManager::CalibrationData& CalibrationManager::data() const noexcept { return data_; }

bool CalibrationManager::applyCalibration() {
    if (audioInput_ == nullptr || !data_.isValid) {
        return false;
    }
    audioInput_->setGainDb(audioInput_->getGainDb() + data_.optimalGainDb);
    return true;
}

std::string CalibrationManager::infoString() const {
    std::ostringstream stream;
    stream << "Noise: " << data_.noiseFloorDb << " dB | Gain: " << data_.optimalGainDb
           << " dB | Peak: " << data_.referencePeakDb << " dB";
    return stream.str();
}

void CalibrationManager::ingestNoiseSamples(const std::vector<std::int16_t>& samples) {
    for (auto sample : samples) {
        if (calibrationBufferPos_ >= calibrationBuffer_.size()) {
            calibrationBufferPos_ = 0;
        }
        calibrationBuffer_[calibrationBufferPos_++] = sample;
    }

    if (calibrationBufferPos_ >= calibrationBuffer_.size()) {
        float absSum = 0.0f;
        float sqSum = 0.0f;
        for (auto sample : calibrationBuffer_) {
            const float normalized = static_cast<float>(sample) / 32767.0f;
            absSum += std::abs(normalized);
            sqSum += normalized * normalized;
        }
        const float meanAbs = absSum / static_cast<float>(calibrationBuffer_.size());
        const float meanSq = sqSum / static_cast<float>(calibrationBuffer_.size());
        const float variance = std::max(0.0f, meanSq - meanAbs * meanAbs);
        data_.noiseFloorDb = amplitudeToDb(std::sqrt(meanSq));
        data_.noiseStdDev = std::sqrt(variance);
    }
}

void CalibrationManager::ingestReferenceSamples(const std::vector<std::int16_t>& samples) {
    for (auto sample : samples) {
        if (calibrationBufferPos_ < calibrationBuffer_.size()) {
            calibrationBuffer_[calibrationBufferPos_++] = sample;
        }
    }
}

void CalibrationManager::analyzeResults(std::uint64_t timestampMs) {
    if (calibrationBufferPos_ == 0) {
        state_ = CalibrationState::Error;
        return;
    }
    std::int16_t peakSample = 0;
    for (std::size_t i = 0; i < calibrationBufferPos_; ++i) {
        if (std::abs(calibrationBuffer_[i]) > std::abs(peakSample)) {
            peakSample = calibrationBuffer_[i];
        }
    }
    const float peakLinear = std::abs(static_cast<float>(peakSample)) / 32767.0f;
    data_.referencePeakDb = amplitudeToDb(peakLinear);
    data_.optimalGainDb = clampFloat(config_.targetPeakDb - data_.referencePeakDb,
                                     config_.minGainDb, config_.maxGainDb);

    const std::size_t bins = data_.frequencyResponse.size();
    if (bins > 0) {
        const std::size_t chunk = std::max<std::size_t>(1, calibrationBufferPos_ / bins);
        for (std::size_t bin = 0; bin < bins; ++bin) {
            const std::size_t start = bin * chunk;
            const std::size_t end = std::min(calibrationBufferPos_, start + chunk);
            float energy = 0.0f;
            for (std::size_t i = start; i < end; ++i) {
                const float normalized = static_cast<float>(calibrationBuffer_[i]) / 32767.0f;
                energy += normalized * normalized;
            }
            const float rms = end > start ? std::sqrt(energy / static_cast<float>(end - start)) : 0.0f;
            data_.frequencyResponse[bin] = amplitudeToDb(rms);
        }
    }

    data_.calibrationTimestampMs = timestampMs;
    data_.isValid = true;
}

void CalibrationManager::reset() {
    state_ = CalibrationState::Idle;
    progress_ = 0;
    startTimeMs_ = 0;
    calibrationBufferPos_ = 0;
    std::fill(calibrationBuffer_.begin(), calibrationBuffer_.end(), 0);
    data_ = {};
    data_.frequencyResponse.assign(config_.frequencyResponseBins, 0.0f);
}

void CalibrationManager::measureNoiseFloor() {
    if (audioInput_ == nullptr) {
        return;
    }
    ingestNoiseSamples(audioInput_->readSamples(256));
}

void CalibrationManager::captureReference() {
    if (audioInput_ == nullptr) {
        return;
    }
    ingestReferenceSamples(audioInput_->readSamples(256));
}

float CalibrationManager::amplitudeToDb(float value) {
    return value > 0.0f ? 20.0f * std::log10(value) : -80.0f;
}

}  // namespace coil_analyzer_core

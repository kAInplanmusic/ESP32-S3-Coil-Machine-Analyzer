#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "coil_analyzer_core/AudioInput.h"
#include "coil_analyzer_core/CoreConfig.h"
#include "coil_analyzer_core/Logger.h"

namespace coil_analyzer_core {

class CalibrationManager {
public:
    enum class CalibrationState : std::uint8_t {
        Idle,
        MeasuringNoise,
        CapturingReference,
        Analyzing,
        Complete,
        Error,
    };

    struct CalibrationData {
        float noiseFloorDb = 0.0f;
        float noiseStdDev = 0.0f;
        float optimalGainDb = 0.0f;
        float referencePeakDb = 0.0f;
        std::vector<float> frequencyResponse;
        std::uint64_t calibrationTimestampMs = 0;
        bool isValid = false;
    };

    explicit CalibrationManager(CalibrationConfig config = {}, Logger* logger = nullptr);

    void attachAudioInput(AudioInput* audioInput);
    void startCalibration(std::uint32_t durationMs = 5000);
    bool update(std::uint64_t currentTimeMs);
    CalibrationState state() const noexcept;
    std::uint8_t progress() const noexcept;
    const CalibrationData& data() const noexcept;
    bool applyCalibration();
    std::string infoString() const;
    void ingestNoiseSamples(const std::vector<std::int16_t>& samples);
    void ingestReferenceSamples(const std::vector<std::int16_t>& samples);
    void analyzeResults(std::uint64_t timestampMs);
    void reset();

private:
    void measureNoiseFloor();
    void captureReference();
    static float amplitudeToDb(float value);

    CalibrationConfig config_;
    Logger* logger_;
    NullLogger fallbackLogger_;
    AudioInput* audioInput_ = nullptr;
    CalibrationState state_ = CalibrationState::Idle;
    CalibrationData data_;
    std::uint8_t progress_ = 0;
    std::uint64_t startTimeMs_ = 0;
    std::uint32_t durationMs_ = 5000;
    std::vector<std::int16_t> calibrationBuffer_;
    std::size_t calibrationBufferPos_ = 0;
};

}  // namespace coil_analyzer_core

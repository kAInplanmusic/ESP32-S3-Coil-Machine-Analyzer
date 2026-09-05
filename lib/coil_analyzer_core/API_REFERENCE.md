# coil_analyzer_core API Reference

`coil_analyzer_core` is a portable C++17 library that extracts the ESP32-S3 Coil Machine Analyzer's core signal-analysis logic into a hardware-agnostic package.

## Directory Layout

- `include/coil_analyzer_core/` - Public API
- `src/` - Core implementations
- `tests/` - Portable unit tests
- `CMakeLists.txt` - Standalone library/test build

## Core Components

### `SignalProcessor`
Portable FFT, waveform buffering, spectrum peak detection, bandpass filtering, CPS estimation, and harmonic extraction.

#### Key types
- `SignalProcessorConfig`
- `SignalProcessor::SpectrumData`
- `SignalProcessor::ImpactEvent`

#### Example
```cpp
#include "coil_analyzer_core/SignalProcessor.h"
using namespace coil_analyzer_core;

SignalProcessor processor;
processor.processSamples(audioBlock, timestampMicros);
processor.computeFft();
auto peaks = processor.findSpectrumPeaks(processor.spectrumData().magnitudeDb, -30.0f);
```

### `AdvancedImpactAnalyzer`
Portable multi-point impulse segmentation and waveform characterization, including extrema extraction, decay estimation, harmonic ratios, and quality metrics.

#### Key types
- `ImpactAnalyzerConfig`
- `AdvancedImpactAnalyzer::MeasurementPoint`
- `AdvancedImpactAnalyzer::ImpactWaveform`
- `AdvancedImpactAnalyzer::AdvancedStats`

#### Example
```cpp
#include "coil_analyzer_core/AdvancedImpactAnalyzer.h"
using namespace coil_analyzer_core;

AdvancedImpactAnalyzer analyzer;
analyzer.startMeasurement();
for (std::uint32_t i = 0; i < samples.size(); ++i) {
    analyzer.processSample(samples[i], i, timestampMs);
}
if (analyzer.stopMeasurement()) {
    auto quality = analyzer.qualityScore();
}
```

### `CalibrationManager`
Portable calibration pipeline for noise-floor estimation, reference-capture analysis, gain recommendation, and coarse frequency-response profiling.

#### Key types
- `CalibrationConfig`
- `CalibrationManager::CalibrationData`
- `AudioInput`

#### Example
```cpp
#include "coil_analyzer_core/CalibrationManager.h"
using namespace coil_analyzer_core;

class MyAudioInput : public AudioInput {
public:
    std::size_t readSamples(std::int16_t* dst, std::size_t max) override;
    float getGainDb() const override;
    void setGainDb(float gainDb) override;
};

MyAudioInput input;
CalibrationManager calibration;
calibration.attachAudioInput(&input);
calibration.startCalibration(5000);
calibration.update(nowMs);
```

## Hardware Abstraction Interfaces

### `AudioInput`
Abstract pull-based PCM provider with gain control hooks.

### `DataFormatter`
Abstract serialization interface for JSON, CSV, or platform-specific formats.

### `Logger`
Abstract logging interface. `NullLogger` is provided as a no-op default.

## Build

```bash
cmake -S lib/coil_analyzer_core -B lib/coil_analyzer_core/build
cmake --build lib/coil_analyzer_core/build
ctest --test-dir lib/coil_analyzer_core/build --output-on-failure
```

## Notes

- No Arduino, ESP-IDF, EEPROM, or CMSIS dependencies remain in the new library.
- Standard C++ containers replace embedded-specific abstractions.
- Existing firmware files were left untouched; this library is additive.

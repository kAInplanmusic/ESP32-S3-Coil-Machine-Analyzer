# Core Library - libcoilanalyzer

Platform-agnostic C++17 library for coil machine waveform analysis.

## Overview

This library contains all the signal processing and analysis logic, independent of:
- Audio input hardware/API
- Display technology
- Operating system

It can be compiled for:
- **Desktop (Windows/Linux)**: Native C++ library
- **Web**: WebAssembly via Emscripten
- **Mobile (Android)**: Via JNI bindings
- **Embedded (ESP32)**: Arduino-compatible

## Architecture

```
libcoilanalyzer/
├── include/
│   ├── signal_processor.h       # FFT, peak detection
│   ├── impact_analyzer.h        # Waveform analysis
│   └── data_models.h            # Serialization
├── src/
│   ├── signal_processor.cpp
│   ├── impact_analyzer.cpp
│   └── data_models.cpp
├── tests/
│   ├── test_signal_processor.cpp
│   ├── test_impact_analyzer.cpp
│   └── CMakeLists.txt
└── CMakeLists.txt
```

## Components

### 1. SignalProcessor

Handles low-level signal processing:
- FFT computation (Hann window)
- Peak detection
- Harmonic detection
- RMS and peak-to-peak calculation

```cpp
coilanalyzer::SignalProcessor processor;
processor.init();

// Process samples
int16_t samples[4096];
processor.process_samples(samples, 4096);

// Compute FFT
if (processor.compute_fft()) {
  processor.detect_peaks();
  const auto& events = processor.get_impact_events();
}
```

### 2. AdvancedImpactAnalyzer

Multi-point waveform analysis:
- Impact detection and cycle extraction
- Extrema finding (peaks/valleys)
- Harmonic ratio calculation
- Decay rate analysis
- Quality scoring

```cpp
coilanalyzer::AdvancedImpactAnalyzer analyzer;
analyzer.init();
analyzer.start_measurement();

// Process samples
for (uint32_t i = 0; i < sample_count; ++i) {
  analyzer.process_sample(samples[i], i);
}

analyzer.stop_measurement();
const auto& stats = analyzer.get_stats();
```

### 3. DataModels

Serialization and export:
- MeasurementResult: Complete analysis output
- CalibrationData: Microphone calibration
- AudioConfig: Audio input settings
- MeasurementSession: Full measurement session

```cpp
auto result = analyzer.get_stats().to_measurement_result();
std::string json = result.to_json();
std::string csv = result.export_waveform_csv();
```

## Building

### Linux/Windows (CMake)

```bash
cd libcoilanalyzer
cmake -B build
cd build
make
sudo make install
```

### WebAssembly (Emscripten)

```bash
# Install Emscripten (if not installed)
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
./emsdk install latest
./emsdk activate latest
source ./emsdk_env.sh

# Build WASM
cd libcoilanalyzer
cmake -B build-wasm \
  -DCMAKE_TOOLCHAIN_FILE=$EMSDK/cmake/Emscripten.cmake \
  -DBUILD_WASM=ON
cd build-wasm
make

# Output: libcoilanalyzer.js, libcoilanalyzer.wasm
```

### Android (NDK)

```bash
# Generate bindings
cd android-bindings
./gradlew build

# Outputs: libcoilanalyzer.so for ARM64
```

## Testing

```bash
cmake -B build -DBUILD_TESTS=ON
cd build
make
ctest --output-on-failure
```

### Test Coverage

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Coverage
cd build
make
make coverage  # Generates coverage report
```

## API Reference

### SignalProcessor

```cpp
class SignalProcessor {
public:
  // Initialization
  bool init();
  
  // Audio processing
  void process_samples(const int16_t* samples, size_t count);
  bool compute_fft();
  bool detect_peaks(size_t min_distance, float threshold);
  
  // Data retrieval
  const std::vector<ImpactEvent>& get_impact_events() const;
  const SpectrumData& get_spectrum_data() const;
  float calculate_cps() const;
  float get_rms() const;
  float get_peak_to_peak() const;
};
```

### AdvancedImpactAnalyzer

```cpp
class AdvancedImpactAnalyzer {
public:
  // Control
  void start_measurement();
  void stop_measurement();
  void reset();
  
  // Processing
  void process_sample(int16_t sample, uint32_t sample_index);
  
  // Results
  bool is_measuring() const;
  uint8_t get_measurement_progress() const;
  const ImpactWaveform& get_latest_waveform() const;
  const AdvancedStats& get_stats() const;
  float get_cps() const;
};
```

### MeasurementResult

```cpp
class MeasurementResult {
public:
  // Export
  std::string to_json() const;
  std::string to_csv() const;
  std::string export_waveform_csv() const;
  std::string export_harmonics_csv() const;
  
  // Members
  float average_cps;
  uint8_t overall_quality_score;
  std::vector<float> harmonic_frequencies;
  std::vector<float> harmonic_amplitudes;
};
```

## Performance Characteristics

| Operation | Time | Memory |
|-----------|------|--------|
| FFT (2048 pts) | <50ms | ~50KB |
| Peak detection | <10ms | ~10KB |
| 100 cycle analysis | <5s | ~500KB |
| JSON export | <50ms | file-dependent |

## Integration Examples

### C++

```cpp
#include "coilanalyzer/signal_processor.h"
#include "coilanalyzer/impact_analyzer.h"

using namespace coilanalyzer;

int main() {
  SignalProcessor processor;
  AdvancedImpactAnalyzer analyzer;
  
  processor.init();
  analyzer.init();
  
  // Process audio...
  analyzer.start_measurement();
  // ... feed samples ...
  analyzer.stop_measurement();
  
  auto result = analyzer.get_stats();
  std::cout << "Quality: " << (int)result.overall_quality_score << "%\n";
}
```

### Python (via ctypes)

```python
import ctypes

lib = ctypes.CDLL('./libcoilanalyzer.so')

# Process samples
processor = lib.SignalProcessor_new()
lib.SignalProcessor_init(processor)

# Call C++ functions
samples = (ctypes.c_int16 * len(audio_data))(*audio_data)
lib.SignalProcessor_process_samples(processor, samples, len(audio_data))
```

### WebAssembly (JavaScript)

```javascript
const Module = await CreateModule();

// Create processor
const processor = new Module.SignalProcessor();
processor.init();

// Process samples
const samples = new Int16Array(audioData);
processor.process_samples(samples.byteOffset, samples.length);

// Get results
const events = processor.get_impact_events();
```

## Dependencies

- **Standard Library**: No external C++ dependencies
- **Optional**: kissfft for FFT (included), Eigen for matrix operations

## Building Documentation

```bash
# Install Doxygen
sudo apt-get install doxygen graphviz

# Generate documentation
cd libcoilanalyzer
doxygen Doxyfile

# Output: docs/html/index.html
```

## Contributing

See ../CONTRIBUTING.md

## License

MIT License - See ../LICENSE

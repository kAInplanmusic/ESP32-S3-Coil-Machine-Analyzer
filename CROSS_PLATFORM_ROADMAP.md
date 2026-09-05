# Cross-Platform Coil Machine Analyzer

## Overview
This document outlines the strategy to convert the ESP32-S3 embedded firmware into a fully cross-platform application suite supporting Windows, Linux, Android, and Web browsers.

## Architecture

### Core Layer (Hardware-Agnostic)
```
libcoilanalyzer/
├── signal_processing/      # FFT, filtering, peak detection
├── impact_analysis/        # Waveform extraction, harmonics, decay
├── measurement_engine/     # Statistics, quality scoring
├── data_models/           # Measurement structures, export formats
└── CMakeLists.txt         # Build for Windows/Linux/Android
```

### Platform Layers

#### 1. Web App (HTML5 + TypeScript + WebGL)
```
web-app/
├── public/
├── src/
│   ├── components/        # React components
│   ├── pages/            # Main views
│   ├── services/         # API services
│   ├── visualizations/   # Canvas/WebGL rendering
│   └── workers/          # Web Workers for processing
├── package.json
└── vite.config.ts
```

**Features:**
- Browser WebAudio API for microphone input
- Real-time waveform visualization (Canvas/WebGL)
- Harmonic analysis display
- Results export (CSV/JSON)
- No server required (PWA-ready)

#### 2. Desktop App (Windows/Linux - Electron + TypeScript)
```
desktop-app/
├── public/
├── src/
│   ├── main/             # Electron main process
│   ├── renderer/         # React UI (shared with web)
│   └── backend/          # Node.js audio processing
├── package.json
└── electron-builder.json
```

**Features:**
- Native audio input (via node-portaudio or SDL2)
- Cross-platform file system access
- Offline operation
- Native menu & file dialogs

#### 3. Android App (React Native or Flutter)
```
mobile-app/
├── src/
│   ├── screens/          # Screen components
│   ├── services/         # Native module wrappers
│   └── navigation/       # Navigation stack
├── android/              # Native Android layer (if React Native)
├── app.json
└── package.json
```

**Features:**
- Native Android microphone access
- Touch-optimized UI
- Offline measurements
- Result sharing & export

### Shared Services
- REST API (optional, for server-based processing)
- Data export formats (CSV, JSON, PDF)
- Measurement serialization

## Development Phases

### Phase 1: Core Library Extraction (Week 1)
- [ ] Extract signal processing algorithms (C++ library)
- [ ] Create platform-agnostic data structures
- [ ] Build CMake build system
- [ ] Write unit tests for core algorithms

### Phase 2: Web App (Week 2)
- [ ] Set up Vite + React + TypeScript
- [ ] Implement WebAudio API wrapper
- [ ] Create waveform visualization (Canvas)
- [ ] Integrate core library (via WebAssembly or asm.js)
- [ ] Add harmonic display and quality scoring UI

### Phase 3: Desktop App (Week 3)
- [ ] Set up Electron + React
- [ ] Implement native audio input (PortAudio)
- [ ] Share React components with web app
- [ ] Add file system operations
- [ ] Distribute as executable

### Phase 4: Android App (Week 4)
- [ ] Choose framework (React Native or Flutter)
- [ ] Implement Android audio recording
- [ ] Create mobile-optimized UI
- [ ] Add measurement list and history
- [ ] Configure app store deployment

### Phase 5: Enhancement & Polish (Week 5)
- [ ] Real-time BLE connection to ESP32 (optional)
- [ ] Advanced export (PDF reports)
- [ ] Performance optimization
- [ ] Testing on multiple devices
- [ ] Documentation and tutorials

## Technology Stack

| Component | Technology | Notes |
|-----------|-----------|-------|
| **Core Library** | C++ 17 + CMake | Cross-platform compilation |
| **Web App** | Vite + React + TypeScript | Fast dev, WASM support |
| **Desktop** | Electron + React + PortAudio | Code reuse with web |
| **Android** | React Native or Flutter | Mobile UI optimization |
| **Audio Input** | WebAudio (Web), PortAudio (Desktop), AAudio (Android) | Platform-specific |
| **Visualization** | Canvas 2D + WebGL | Fast rendering |
| **Testing** | Jest + Vitest | JavaScript testing |
| **Build** | CMake + Webpack/Vite | Cross-platform builds |

## File Structure

```
ESP32-S3-Coil-Machine-Analyzer/
├── firmware/                    # Original ESP32 firmware
│   ├── src/
│   ├── include/
│   └── platformio.ini
│
├── libcoilanalyzer/             # Core C++ library
│   ├── src/
│   │   ├── signal_processor.cpp
│   │   ├── impact_analyzer.cpp
│   │   ├── measurement_engine.cpp
│   │   └── data_models.cpp
│   ├── include/
│   │   ├── signal_processor.h
│   │   ├── impact_analyzer.h
│   │   ├── measurement_engine.h
│   │   └── data_models.h
│   ├── tests/
│   └── CMakeLists.txt
│
├── web-app/                     # Web application
│   ├── src/
│   ├── public/
│   └── package.json
│
├── desktop-app/                 # Electron app (Windows/Linux)
│   ├── public/
│   ├── src/
│   └── package.json
│
├── mobile-app/                  # React Native app (Android)
│   ├── src/
│   ├── android/
│   └── package.json
│
└── README.md
```

## Key Design Decisions

### 1. Core Library Approach
- **Rationale**: Extracting core logic into a reusable C++ library enables:
  - Code reuse across platforms
  - Same algorithms everywhere (consistency)
  - Native performance where needed (desktop)
  - WebAssembly compilation for web

### 2. React for Web + Desktop
- **Rationale**: React allows sharing UI components between web and desktop (via Electron)
- Reduces duplicated effort
- Consistent user experience

### 3. Platform-Specific Audio Input
- **Rationale**: Each platform has native audio APIs optimized for it
  - Web: WebAudio API
  - Desktop: PortAudio (cross-platform) or WASAPI/ALSA
  - Android: AAudio or OpenSL ES

### 4. Offline-First Design
- **Rationale**: Users should be able to record and analyze measurements without network
- Server optional for cloud storage/sharing

## Integration with ESP32 Device

### Future Enhancement: Bluetooth Connection
- Optional Bluetooth Low Energy (BLE) integration
- Send measurements from desktop/mobile to ESP32 for configuration
- Receive real-time measurements from ESP32 device
- Use existing BLE infrastructure from firmware

## Performance Targets

| Metric | Target |
|--------|--------|
| Real-time latency (audio capture to display) | < 100ms |
| FFT computation time (2048 points) | < 50ms |
| UI refresh rate | 10 Hz minimum, 60 Hz ideal |
| Memory usage (per measurement) | < 5MB for 100 cycles |
| File size (web app initial load) | < 2MB gzipped |

## Testing Strategy

### Unit Tests
- Signal processing algorithms (C++)
- Data structure serialization
- Quality scoring calculations

### Integration Tests
- Audio input → Processing → Display pipeline
- File export/import functionality
- Multi-platform consistency

### User Acceptance Testing
- Real coil machine measurements on each platform
- Cross-platform measurement consistency
- User interface usability

## Dependencies

### External Libraries
- **kissfft** (FFT)
- **PortAudio** (Desktop audio input)
- **WASM runtime** (Web FFT/processing)
- **React** (UI framework)
- **Three.js or Babylon.js** (WebGL visualization - optional)

### Build Tools
- CMake (cross-platform builds)
- Webpack/Vite (JavaScript bundling)
- Electron Builder (desktop app distribution)
- React Native CLI (Android)

## Next Steps

1. Extract core algorithms into C++ library
2. Create CMake build configuration
3. Write WebAssembly wrapper for web platform
4. Bootstrap web app with Vite + React
5. Implement WebAudio microphone capture
6. Build initial waveform visualization
7. Iterate through remaining platforms

---

**Status**: Planning phase  
**Last Updated**: 2026-09-05  
**Owner**: Cross-Platform Development Team

# Coil Machine Analyzer - Desktop (Windows/Linux)

Native desktop application for Windows and Linux using Electron and React.

## Features

- **Native Audio Input** - Direct access to system audio devices via PortAudio
- **Real-time Measurement** - Live waveform visualization and analysis
- **Offline Support** - Analyze measurements without internet connection
- **File Management** - Import/export measurements and audio files
- **System Integration** - System tray support, file associations
- **Performance** - Optimized for low-latency audio processing

## Getting Started

### Requirements

- Node.js 16+
- Python 3.8+ (for PortAudio compilation)
- Windows 10+ or Linux (Ubuntu 20.04+)

### Installation

```bash
cd desktop
npm install
```

### Development

```bash
npm run dev
```

This will start both the React dev server and Electron with hot reload.

### Building

#### Windows
```bash
npm run dist:win
```

#### Linux
```bash
npm run dist:linux
```

## Architecture

### Main Process (Electron)
- Handles application lifecycle
- Manages native dialogs (file open/save)
- Provides IPC bridge to renderer
- Manages application window

### Renderer Process (React)
- User interface
- Measurement controls
- Data visualization
- State management

### IPC Communication
- `store-get` / `store-set` - Persistent storage
- `dialog-open-file` - File selection dialog
- `audio-*` - Audio device management

## Audio Processing

### PortAudio Integration
- Multi-platform audio device access
- Low-latency audio capture
- Device enumeration and selection
- Real-time buffer processing

### Processing Pipeline
1. Audio capture from selected device
2. FFT computation (16K samples)
3. Real-time peak detection
4. Waveform display update
5. Database persistence

## File Structure

```
desktop/
├── src/
│   ├── main.ts          # Electron main process
│   ├── preload.ts       # IPC preload script
│   └── App.tsx          # React app
├── public/
│   ├── index.html
│   └── icon.png
├── package.json
└── tsconfig.electron.json
```

## Configuration

### Settings Storage

Settings are stored in:
- **Windows**: `%APPDATA%/Coil Analyzer/config.json`
- **Linux**: `~/.config/Coil Analyzer/config.json`

### Development Settings

Edit `.env`:
```
REACT_APP_API_URL=http://localhost:3001
REACT_APP_WS_URL=ws://localhost:3001
```

## Troubleshooting

### Audio Device Not Found
1. Check Windows/Linux audio settings
2. Ensure microphone is connected
3. Run as administrator on Windows

### Build Fails on Linux
```bash
# Install required build dependencies
sudo apt-get install build-essential python3
```

### Electron Won't Start
```bash
# Clear Electron cache and rebuild
rm -rf node_modules
npm install
npm run dev
```

## Performance

- **Startup Time**: ~3 seconds
- **Audio Latency**: <50ms
- **Memory Usage**: ~200MB
- **CPU Usage**: ~10-20%

## Distribution

Pre-built installers are available for:
- Windows: `.exe` (NSIS installer) and portable `.exe`
- Linux: `.AppImage` and `.deb` package

Download from: https://github.com/kAInplanmusic/ESP32-S3-Coil-Machine-Analyzer/releases

## License

MIT License

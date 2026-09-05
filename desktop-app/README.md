# Desktop Application - Coil Machine Analyzer

Cross-platform desktop application for Windows and Linux with native audio input and file system access.

## Features

- **Desktop App**: Windows (.exe) and Linux (.AppImage) installers
- **Native Audio**: PortAudio for high-quality audio capture
- **File System**: Save measurements locally with full history
- **Offline**: Complete operation without internet connection
- **Dark Theme**: Optimized UI for extended use

## System Requirements

### Windows
- Windows 10 or later (64-bit)
- 100 MB disk space
- Microphone

### Linux
- Ubuntu 20.04+ or equivalent
- 100 MB disk space
- PulseAudio or ALSA
- Microphone

## Quick Start

```bash
cd desktop-app
npm install
npm run dev
```

This starts both the React development server and Electron app.

## Building

### Windows
```bash
npm run build-win
```
Creates `Coil Machine Analyzer Setup 1.0.0.exe`

### Linux
```bash
npm run build-linux
```
Creates `Coil Machine Analyzer-1.0.0.AppImage`

## Architecture

### Main Process (src/main/main.ts)
- Electron application lifecycle
- Window management
- File operations
- Native menu

### Renderer Process (src/renderer/)
- React components (shared with web app)
- DOM rendering
- User interactions

### Backend (src/backend/)
- Audio capture via PortAudio
- Signal processing
- File I/O operations
- IPC communication

## Audio Input

### PortAudio Integration

The desktop app uses PortAudio for cross-platform audio input:

```typescript
import * as portaudio from 'portaudio'

const recorder = new portaudio.AudioRecorder({
  channels: 1,
  sampleRate: 44100,
  frameSize: 4096
})

recorder.on('data', (chunk: Buffer) => {
  // Process audio chunk
})
```

### Supported Devices

- **Windows**: WASAPI, DirectSound
- **Linux**: ALSA, PulseAudio, JACK

## File Storage

Measurements are stored locally:

```
~/.coil-analyzer/
├── measurements/
│   ├── 2024-01-01T10-30-45.json
│   ├── 2024-01-01T11-15-22.json
│   └── ...
├── calibration/
│   └── calibration.json
└── settings.json
```

### Data Format

```json
{
  "id": "measurement-uuid",
  "timestamp": "2024-01-01T10:30:45Z",
  "device": "Microphone Name",
  "result": {
    "cps": 85.5,
    "quality_score": 92,
    "waveform": [...],
    "harmonics": {...}
  }
}
```

## Native Menus

The app includes native menus for:
- File operations (Save, Export, Quit)
- Edit (Undo, Redo, Copy, Paste)
- View (Reload, DevTools)
- Help (About, Documentation)

## Keyboard Shortcuts

| Shortcut | Action |
|----------|--------|
| Ctrl+S   | Save Measurement |
| Ctrl+E   | Export Results |
| Ctrl+Q   | Quit Application |
| Ctrl+R   | Reload |
| Ctrl+Alt+I | Developer Tools |

## Configuration

Settings are stored in `~/.coil-analyzer/settings.json`:

```json
{
  "audio": {
    "device": 0,
    "sampleRate": 44100,
    "bufferSize": 4096
  },
  "ui": {
    "theme": "dark",
    "defaultExportFormat": "json"
  },
  "measurements": {
    "autoSave": true,
    "retentionDays": 90
  }
}
```

## Troubleshooting

### Audio capture not working
```bash
# List available audio devices
./tools/list-devices.js

# Check audio permissions
# Windows: Settings → Privacy → Microphone
# Linux: sudo usermod -a -G audio $USER
```

### High CPU usage
1. Reduce sample rate in settings
2. Close other applications
3. Update audio drivers

### Port already in use
```bash
# Kill process using port 3000
npx kill-port 3000
```

## Development

### HMR (Hot Module Reload)

The app supports HMR for faster development:

```bash
npm run dev
# Changes to src/renderer/ are reflected immediately
# Changes to src/main/ require reload (Ctrl+R)
```

### Building for Development

```bash
npm run build  # Build only (without creating installer)
npm run dev    # Build + Run with DevTools
```

## Distribution

To distribute builds:

1. Create GitHub release
2. Upload installers:
   - `Coil Machine Analyzer Setup 1.0.0.exe`
   - `Coil Machine Analyzer-1.0.0.AppImage`

Users can download and install directly.

## Contributing

See ../CONTRIBUTING.md

## License

MIT License - See ../LICENSE

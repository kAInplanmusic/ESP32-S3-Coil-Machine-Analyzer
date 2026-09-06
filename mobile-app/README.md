# Mobile Application - Coil Machine Analyzer

Native mobile application for Android using React Native with offline functionality.

## Features

- **Native Android App**: Installs like any other Android app
- **Microphone Access**: Native Android audio recording API
- **Offline**: Complete operation without internet connection
- **Touch Optimized**: Full-screen interface for mobile
- **Share Results**: Export and share measurements via email/messaging
- **Measurement History**: Local database of all measurements

## System Requirements

### Android
- Android 8.0 (API 26) or later
- 50 MB storage space
- Microphone

## Quick Start

```bash
cd mobile-app
npm install

# Install on connected device or emulator
npm run android
```

## Building Release APK

```bash
# Debug APK
npm run build:android

# Release APK (requires keystore)
cd android
./gradlew assembleRelease
```

## Architecture

### Navigation

```
Home Screen
  ↓
Measurement Screen (recording)
  ↓
Results Screen (analysis)
```

### Screens

#### HomeScreen
- Display saved measurements
- Start new measurement
- Settings
- Help/About

#### MeasurementScreen
- Live microphone visualization
- Progress indicator
- CPS display
- Quality score
- Stop button

#### ResultsScreen
- Summary metrics
- Waveform visualization
- Harmonic chart
- Decay curve
- Export/Share buttons

## Audio Recording

### Native Module Integration

Audio recording uses native Android API via React Native:

```typescript
import { AudioRecorder } from '../services/audio-recorder'

const recorder = new AudioRecorder({
  sampleRate: 44100,
  bitDepth: 16,
  channels: 1
})

await recorder.start()
// ...
const audioData = await recorder.stop()
```

## Permissions

The app requests:

```xml
<uses-permission android:name="android.permission.RECORD_AUDIO" />
<uses-permission android:name="android.permission.READ_EXTERNAL_STORAGE" />
<uses-permission android:name="android.permission.WRITE_EXTERNAL_STORAGE" />
```

Runtime permission handling (Android 6.0+):

```typescript
import { PermissionsAndroid } from 'react-native'

const granted = await PermissionsAndroid.request(
  PermissionsAndroid.PERMISSIONS.RECORD_AUDIO,
  {
    title: 'Microphone Permission',
    message: 'Need access to your microphone for measurements',
    buttonNeutral: 'Ask Me Later',
    buttonNegative: 'Cancel',
    buttonPositive: 'OK',
  }
)
```

## Local Storage

Measurements stored in app-specific directory:

```
/data/data/com.coil-analyzer.mobile/
├── measurements/
│   └── *.json
├── cache/
└── files/
```

Access via:

```typescript
import { DocumentDirectoryPath } from 'react-native-fs'

const measurementPath = `${DocumentDirectoryPath}/measurements`
```

## Export Functionality

### Share via Intent

```typescript
import { Share } from 'react-native'

Share.share({
  message: JSON.stringify(measurementResult),
  title: 'Measurement Result',
  url: fileUri,
  type: 'application/json'
})
```

### Email Export

```typescript
import { sendMail } from 'react-native-mail'

sendMail({
  subject: 'Coil Machine Measurement',
  recipients: ['user@example.com'],
  body: 'See attached measurement',
  attachmentPath: filePath,
  attachmentType: 'json'
})
```

## User Interface

### Custom Components

- **WaveformDisplay**: Canvas-based waveform visualization
- **ProgressBar**: Circular progress indicator
- **MetricCard**: Displays single metric with label

### Theme

```typescript
export const theme = {
  colors: {
    primary: '#2980b9',
    secondary: '#27ae60',
    danger: '#e74c3c',
    background: '#ecf0f1',
    text: '#2c3e50',
    border: '#bdc3c7'
  },
  spacing: {
    xs: 4,
    sm: 8,
    md: 16,
    lg: 24,
    xl: 32
  }
}
```

## Database

Optional: SQLite for local measurement history

```typescript
import { SQLiteDatabase } from 'react-native-sqlite-storage'

const db = await openDatabase({ name: 'measurements.db' })
await db.executeSql(
  'CREATE TABLE IF NOT EXISTS measurements (id TEXT PRIMARY KEY, data TEXT, timestamp DATETIME)'
)
```

## Testing

```bash
# Unit tests
npm run test

# E2E tests (Detox)
npm run test:e2e
```

## Performance

- Target: 60 FPS during measurement
- Memory: < 100 MB during recording
- Battery: ~1-2% per measurement

### Optimization Tips

1. Use native modules for audio processing
2. Limit UI updates to 10 Hz
3. Offload heavy computation to background thread
4. Compress JSON before storage

## Troubleshooting

### No audio input detected
1. Check app permissions: Settings → Apps → Coil Analyzer → Permissions
2. Ensure microphone is enabled
3. Try rebooting device

### App crashes on start
```bash
# Clear app data
adb shell pm clear com.coil-analyzer.mobile

# Reinstall
npm run android
```

### High battery drain
1. Disable live visualization if not needed
2. Reduce update frequency in settings
3. Close other apps using microphone

## Contributing

See ../CONTRIBUTING.md

## License

MIT License - See ../LICENSE

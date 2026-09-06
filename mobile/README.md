# Coil Machine Analyzer - Mobile (Android/iOS)

Cross-platform mobile application built with React Native and Expo.

## Features

- **Native Audio Capture** - Direct access to device microphones
- **Real-time Measurement** - Live analysis with progress tracking
- **Offline Support** - Full offline functionality with local storage
- **Responsive UI** - Optimized for phones and tablets
- **Dark Mode** - System-aware theming
- **Background Processing** - Measurements continue in background

## Getting Started

### Requirements

- Node.js 16+
- Expo CLI: `npm install -g expo-cli`
- Expo Go app (for testing on device)

### Installation

```bash
cd mobile
npm install
```

### Running

#### Development (Expo Go)
```bash
npm start
```

Scan the QR code with Expo Go app on your phone.

#### Building

**Android**
```bash
npm run android
```

**iOS** (macOS only)
```bash
npm run ios
```

**Web**
```bash
npm run web
```

## Architecture

### Navigation
- Bottom tab navigation for main screens
- Stack navigation for detailed views
- Deep linking support

### Screens

1. **Measurement** - Live audio capture and analysis
2. **History** - List of all measurements
3. **Analysis** - Detailed results view
4. **Settings** - App preferences

### Storage
- AsyncStorage for preferences
- SQLite (via expo-sqlite) for measurements
- File system for audio clips

### Audio
- Expo.Audio for recording
- Native Android/iOS audio APIs
- Real-time processing with Web Audio API (web)

## File Structure

```
mobile/
├── src/
│   ├── screens/
│   │   ├── MeasurementScreen.tsx
│   │   ├── HistoryScreen.tsx
│   │   ├── AnalysisScreen.tsx
│   │   └── SettingsScreen.tsx
│   ├── components/
│   ├── services/
│   ├── hooks/
│   └── navigation/
├── App.tsx
├── app.json
├── package.json
└── tsconfig.json
```

## Permissions

### Android
- `RECORD_AUDIO` - Microphone access
- `READ_EXTERNAL_STORAGE` - File access
- `WRITE_EXTERNAL_STORAGE` - Save measurements

### iOS
- `NSMicrophoneUsageDescription` - Microphone access
- `NSPhotoLibraryUsageDescription` - Photo access

## Performance

- **Startup Time**: 2-3 seconds
- **Audio Latency**: <100ms
- **Memory Usage**: 100-150MB
- **Battery**: ~20% per hour of recording

## Testing

```bash
npm test
```

## Building APK/IPA

### Android
```bash
expo build:android -t apk
```

### iOS
```bash
expo build:ios
```

## Distribution

- **Google Play**: Follow Play Store publishing guide
- **App Store**: Follow Apple App Store review guidelines
- **Direct Distribution**: APK/IPA files available from Expo

## Troubleshooting

### Microphone Not Working
1. Check app permissions in settings
2. Restart app after granting permissions
3. Try a different audio source

### Expo Go Connection Issues
```bash
# Use LAN connection
expo start --lan

# Or tunnel (slower but works anywhere)
expo start --tunnel
```

## License

MIT License

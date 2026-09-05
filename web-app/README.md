# Web Application - Coil Machine Analyzer

Cross-platform web-based analyzer using browser WebAudio API for microphone input.

## Features

- **Browser-Based**: No installation required - runs in Chrome, Firefox, Safari, Edge
- **Real-Time Analysis**: Live waveform capture and processing
- **Offline-Ready**: PWA support for offline measurements
- **Export Options**: JSON and CSV formats for data analysis
- **Responsive Design**: Works on desktop and tablet browsers

## Prerequisites

- Node.js 16+ and npm/yarn
- Modern web browser with WebAudio API support
- Microphone access permission

## Quick Start

```bash
cd web-app
npm install
npm run dev
```

The application will start on http://localhost:3000

## Architecture

### Components

- **MicrophoneCapture**: Handles microphone permission and recording
- **MeasurementDisplay**: Shows real-time progress during measurement
- **ResultsPanel**: Displays results with multiple tabs (Summary, Waveform, Harmonics, Decay)

### Services

- **AudioCapture**: WebAudio API wrapper for microphone input
- **SignalProcessor**: FFT and peak detection (integrates with libcoilanalyzer)
- **MeasurementEngine**: Analysis pipeline

### Visualization

- Canvas 2D for waveform and harmonic displays
- WebGL (optional) for advanced visualizations

## Build

```bash
npm run build
```

Outputs optimized build to `dist/` directory.

## Testing

```bash
npm run test
```

## Deployment

### Docker

```dockerfile
FROM node:18-alpine
WORKDIR /app
COPY package*.json ./
RUN npm ci
COPY . .
RUN npm run build
EXPOSE 3000
CMD ["npx", "serve", "-s", "dist"]
```

### Static Hosting

The built application can be deployed to any static hosting provider:
- Vercel
- Netlify
- GitHub Pages
- AWS S3 + CloudFront
- Cloudflare Pages

## Browser Compatibility

| Browser | Support |
|---------|---------|
| Chrome  | ✅ Full |
| Firefox | ✅ Full |
| Safari  | ✅ Full |
| Edge    | ✅ Full |

## API Integration

The web app can connect to a backend API for:
- Cloud storage of measurements
- Advanced analytics
- Batch processing

## WebAssembly (Optional)

For better performance, the core library can be compiled to WebAssembly:

```bash
# In libcoilanalyzer/
emcmake cmake -B build-wasm -DCMAKE_TOOLCHAIN_FILE=$EMSDK/cmake/Emscripten.cmake
cd build-wasm
make
```

Then import in web app:

```typescript
import { SignalProcessor } from '../libcoilanalyzer/build-wasm/signal_processor.js'
```

## Troubleshooting

### Microphone not detected
1. Check browser permissions (Settings → Privacy → Microphone)
2. Ensure page is served over HTTPS (except localhost)
3. Try a different browser

### Poor performance
1. Close other browser tabs
2. Reduce FFT size in settings
3. Use WebAssembly build for better performance

## Contributing

See ../CONTRIBUTING.md

## License

MIT License - See ../LICENSE

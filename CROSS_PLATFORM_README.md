# ESP32-S3 Coil Machine Analyzer - Cross-Platform Implementation

Complete multi-platform architecture for the Coil Machine Analyzer with support for Windows, Linux, Android, and Web applications.

## 🏗️ Architecture Overview

This project has been refactored from a single embedded firmware application into a distributed system with:

1. **Backend Service** - REST API + WebSocket server for measurement processing and data management
2. **Web Application** - React-based web UI for analysis and visualization
3. **Desktop Application** - Windows/Linux support (Electron-based)
4. **Mobile Application** - Android/iOS support (React Native)
5. **Core Analysis Library** - Hardware-agnostic C++ library for signal processing algorithms

## 📁 Project Structure

```
ESP32-S3-Coil-Machine-Analyzer/
├── backend/                      # Node.js/Express REST API server
│   ├── src/
│   │   ├── server.js            # Express app setup
│   │   ├── database/db.js        # SQLite integration
│   │   ├── routes/              # API endpoints
│   │   ├── websocket/           # WebSocket handlers
│   │   ├── middleware/          # Express middleware
│   │   └── utils/               # Utilities (logger, etc)
│   ├── package.json
│   ├── .env.example
│   └── README.md
│
├── web/                          # React web application
│   ├── src/
│   │   ├── pages/               # Page components
│   │   ├── components/          # Reusable components
│   │   ├── services/            # API client
│   │   ├── hooks/               # Custom React hooks
│   │   ├── styles/              # CSS stylesheets
│   │   ├── App.jsx
│   │   └── main.jsx
│   ├── vite.config.js
│   ├── package.json
│   └── README.md
│
├── desktop/                      # Electron desktop app (Windows/Linux)
│   ├── src/
│   ├── public/
│   ├── package.json
│   └── README.md
│
├── mobile/                       # React Native mobile app (Android)
│   ├── app.json
│   ├── package.json
│   └── README.md
│
├── lib/                          # Shared core analysis library
│   └── coil_analyzer_core/
│       ├── include/             # Public headers
│       ├── src/                 # Implementation
│       ├── tests/               # Unit tests
│       ├── CMakeLists.txt       # Build configuration
│       └── README.md
│
├── src/ (firmware)              # Original ESP32 firmware
│   └── ... (embedded code)
│
├── ARCHITECTURE.md              # Detailed architecture documentation
├── CROSS_PLATFORM_GUIDE.md      # Setup guide for all platforms
└── docker-compose.yml           # Docker orchestration
```

## 🚀 Quick Start

### Backend Server (Port 3001)

```bash
cd backend
npm install
cp .env.example .env
npm start
```

**API Endpoints:**
- `GET /api/v1/measurements` - List all measurements
- `POST /api/v1/measurements` - Create new measurement
- `GET /api/v1/measurements/:id` - Get measurement details
- `DELETE /api/v1/measurements/:id` - Delete measurement
- `GET /api/v1/export/:id/csv` - Export as CSV
- `GET /api/v1/export/:id/json` - Export as JSON

**WebSocket:**
- `ws://localhost:3001/ws/measurements` - Real-time measurement streaming

### Web App (Port 5173)

```bash
cd web
npm install
npm run dev
```

Open http://localhost:5173 in your browser

**Features:**
- Live measurement with progress tracking
- Measurement history/table
- Detailed analysis view
- Dark/light theme toggle
- Responsive design for mobile

## 🔧 Technology Stack

### Backend
- **Runtime**: Node.js 18+
- **Framework**: Express.js
- **Database**: SQLite3
- **WebSocket**: ws (native WebSocket support)
- **Deployment**: Docker/Docker Compose

### Frontend (Web)
- **Framework**: React 18
- **Build Tool**: Vite
- **Routing**: React Router v6
- **Charts**: Chart.js + react-chartjs-2
- **HTTP Client**: Axios
- **Styling**: CSS with CSS variables for theme

### Core Library
- **Language**: C++17 (hardware-agnostic)
- **Build**: CMake
- **Testing**: Google Test framework

### Desktop (Planned)
- **Framework**: Electron
- **Audio**: PortAudio / WASAPI
- **UI**: React + Electron IPC

### Mobile (Planned)
- **Framework**: React Native
- **Audio**: Native Android/iOS APIs
- **Storage**: AsyncStorage / SQLite

## 📊 API Documentation

### Create Measurement

```bash
POST /api/v1/measurements
Content-Type: application/json

{
  "cps": 85.3,
  "qualityScore": 92,
  "durationMs": 15000,
  "sampleCount": 660000,
  "data": {
    "harmonicRatioH2": 0.45,
    "harmonicRatioH3": 0.12,
    "thd": 48.5,
    "decayRate": 0.0234,
    "qFactor": 134.5
  },
  "waveforms": [
    {
      "cycleNumber": 1,
      "timestampMs": 0,
      "points": [...],
      "harmonicRatioH2": 0.46,
      "harmonicRatioH3": 0.13,
      "decayRate": 0.0235
    }
  ]
}
```

### WebSocket Message Format

```json
{
  "type": "MEASUREMENT_START|AUDIO_DATA|MEASUREMENT_END|PING",
  "payload": {
    "measurementId": "uuid",
    "data": [...],
    "result": {...}
  }
}
```

## 🐳 Docker Deployment

```bash
docker-compose up
```

Services:
- Backend: http://localhost:3001
- Web UI: http://localhost:5173
- Database: SQLite in container volume

## 📱 Features by Platform

| Feature | Backend | Web | Desktop | Mobile | ESP32 |
|---------|---------|-----|---------|--------|-------|
| Real-time Measurement | ✅ | ✅ | ✅ | ✅ | ✅ |
| Data Persistence | ✅ | ✅ | ✅ | ✅ | ✅ |
| Visualization | ✅ | ✅ | ✅ | ✅ | ✅ |
| Export (CSV/JSON) | ✅ | ✅ | ✅ | ✅ | - |
| Offline Mode | - | - | ✅ | ✅ | ✅ |
| System Tray | - | - | ✅ | - | - |
| BLE Control | - | - | - | ✅ | ✅ |

## 🔐 Security

- CORS enabled for frontend
- Input validation on all endpoints
- SQLite database with parameterized queries
- WebSocket authentication (optional)
- Helmet.js for HTTP security headers

## 📈 Performance

- FFT processing: <50ms per cycle
- Database queries: indexed for fast retrieval
- Frontend: lazy loading, code splitting
- Backend: connection pooling, compression

## 🧪 Testing

```bash
# Backend tests
cd backend && npm test

# Frontend tests
cd web && npm test

# Core library tests
cd lib/coil_analyzer_core && cmake build && ctest
```

## 📚 Documentation

- [Architecture Details](./ARCHITECTURE.md)
- [Cross-Platform Setup Guide](./CROSS_PLATFORM_GUIDE.md)
- [API Reference](./backend/README.md)
- [Web App Documentation](./web/README.md)
- [Core Library](./lib/coil_analyzer_core/README.md)

## 🛠️ Development Workflow

1. **Local Development**
   ```bash
   # Terminal 1: Backend
   cd backend && npm run dev
   
   # Terminal 2: Web
   cd web && npm run dev
   ```

2. **Docker Development**
   ```bash
   docker-compose -f docker-compose.dev.yml up
   ```

3. **Build for Production**
   ```bash
   # Backend
   cd backend && npm run build
   
   # Web
   cd web && npm run build
   
   # Desktop/Mobile: See respective READMEs
   ```

## 📝 License

MIT License - See LICENSE file

## 👤 Author

Developed for tattoo coil machine impact analysis and quality control.

---

**Version**: 3.0 (Cross-Platform)  
**Last Updated**: 2026-09-05

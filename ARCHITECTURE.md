# Cross-Platform Architecture Documentation

## System Overview

The ESP32-S3 Coil Machine Analyzer has been refactored from a single embedded firmware project into a distributed, cross-platform system that maintains the same analysis capabilities across multiple platforms.

```
┌─────────────────────────────────────────────────────────────────┐
│                     User Interface Layer                         │
├────────────────┬──────────────────┬─────────────┬──────────────┤
│  Web Browser   │  Desktop App     │ Mobile App  │  ESP32 Device │
│  (React)       │  (Electron)      │  (React Nav)│  (Firmware)   │
└────────┬───────┴────────┬─────────┴──────┬──────┴────────┬──────┘
         │                │                │               │
         └────────────────┼────────────────┼───────────────┘
                          │
                          ▼
         ┌────────────────────────────────┐
         │   REST API + WebSocket Server   │
         │    (Node.js/Express Backend)    │
         ├────────────────────────────────┤
         │ • Measurement CRUD             │
         │ • Real-time Streaming          │
         │ • Data Export                  │
         │ • Authentication               │
         └────────────────┬───────────────┘
                          │
        ┌─────────────────┼─────────────────┐
        ▼                 ▼                 ▼
    ┌─────────┐      ┌─────────┐      ┌──────────┐
    │ SQLite  │      │ Analysis│      │ Logging  │
    │Database │      │ Engine  │      │  System  │
    └─────────┘      └─────────┘      └──────────┘
                          │
                          ▼
        ┌────────────────────────────────┐
        │  Core Analysis Library (C++)    │
        ├────────────────────────────────┤
        │ • FFT Computation              │
        │ • Waveform Analysis            │
        │ • Harmonic Detection           │
        │ • Quality Scoring              │
        │ • Decay Rate Calculation       │
        └────────────────────────────────┘
```

## Component Details

### 1. Backend Service (Node.js/Express)

**Location**: `./backend/`

**Responsibilities**:
- Expose REST API for measurement operations
- Handle WebSocket connections for real-time data
- Manage database operations
- Provide data export (CSV, JSON)
- User authentication and authorization
- Rate limiting and security

**Key Modules**:
- `server.js` - Express app initialization
- `database/db.js` - SQLite connection and operations
- `routes/measurements.js` - CRUD operations
- `routes/analysis.js` - Analysis endpoints
- `routes/export.js` - Export functionality
- `websocket/ws-handler.js` - WebSocket message handling

**Database Schema**:
```
measurements (id, created_at, cps, quality_score, duration_ms, data_json)
waveforms (id, measurement_id, cycle_number, points_json, harmonic_ratios)
sessions (id, device_info_json, calibration_data_json)
```

### 2. Web Application (React + Vite)

**Location**: `./web/`

**Responsibilities**:
- Browser-based UI for measurement control
- Real-time visualization of waveforms
- Measurement history management
- Data visualization and analysis
- Responsive design for all screen sizes

**Key Components**:
- `App.jsx` - Main application shell
- `pages/MeasurementPage.jsx` - Live measurement interface
- `pages/HistoryPage.jsx` - Measurement history
- `pages/AnalysisPage.jsx` - Detailed analysis view
- `components/WaveformDisplay.jsx` - Waveform visualization
- `components/QualityIndicator.jsx` - Quality score display

**Services**:
- `services/api.js` - REST API client
- `hooks/useAudioCapture.js` - Web Audio API integration

### 3. Core Analysis Library (C++)

**Location**: `./lib/coil_analyzer_core/`

**Responsibilities**:
- Hardware-agnostic signal processing algorithms
- FFT computation
- Peak detection and waveform analysis
- Harmonic ratio calculation
- Quality scoring
- Decay rate estimation

**Key Classes**:
- `SignalProcessor` - FFT and spectral analysis
- `AdvancedImpactAnalyzer` - Multi-point waveform extraction
- `CalibrationManager` - Calibration algorithms
- `DataFormatter` - Results serialization

### 4. Desktop Application (Electron)

**Location**: `./desktop/`

**Responsibilities**:
- Native Windows/Linux UI
- Direct audio device access
- File system integration
- System tray features
- Offline functionality

### 5. Mobile Application (React Native)

**Location**: `./mobile/`

**Responsibilities**:
- Android/iOS native UI
- Mobile audio input
- Background measurement
- Bluetooth control of ESP32
- Push notifications

## Data Flow Architecture

### Measurement Process

```
1. Start Measurement (User clicks)
   └─> Frontend sends MEASUREMENT_START via WebSocket
   
2. Audio Capture
   └─> Audio input → FFT → Feature extraction
   └─> Periodically send AUDIO_DATA chunks
   
3. Analysis
   └─> Backend processes audio samples
   └─> Core library computes metrics:
       - CPS (cycles per second)
       - Harmonic ratios
       - Decay rates
       - Quality score
   
4. Results Storage
   └─> Save to SQLite database
   └─> Send MEASUREMENT_END with results
   
5. Visualization
   └─> Frontend receives results
   └─> Render charts and metrics
```

### Real-Time Streaming Protocol

**WebSocket Messages**:

```json
// Client → Server
{
  "type": "MEASUREMENT_START",
  "payload": {
    "measurementId": "uuid-v4",
    "metadata": {...}
  }
}

// Server → Client (Streaming)
{
  "type": "AUDIO_CHUNK_ACK",
  "payload": {
    "chunkId": 1,
    "cps": 85.2,
    "quality": 91,
    "progress": 45
  }
}

// Client → Server
{
  "type": "MEASUREMENT_END",
  "payload": {
    "measurementId": "uuid-v4",
    "totalSamples": 660000
  }
}

// Server → Client (Final)
{
  "type": "MEASUREMENT_COMPLETED",
  "payload": {
    "measurement": {
      "id": "uuid-v4",
      "cps": 85.3,
      "qualityScore": 92,
      "waveforms": [...]
    }
  }
}
```

## Authentication & Security

### Current Implementation
- CORS enabled for frontend origins
- Helmet.js for HTTP security headers
- Input validation on all endpoints
- Parameterized SQL queries (SQLi prevention)

### Future Enhancements
- JWT-based authentication
- OAuth2 integration
- API key management
- Rate limiting per user/IP
- Encryption of sensitive data

## Deployment Strategies

### Local Development
```bash
# Terminal 1
cd backend && npm run dev

# Terminal 2
cd web && npm run dev
```

### Docker Compose (All-in-one)
```bash
docker-compose up
# Backend: http://localhost:3001
# Web: http://localhost:5173
```

### Cloud Deployment (AWS Example)
- Backend: AWS Lambda + RDS
- Frontend: AWS S3 + CloudFront
- Database: AWS RDS PostgreSQL
- WebSocket: AWS API Gateway

## Performance Considerations

### Backend
- Database indexing on measurement_id, created_at
- Connection pooling (planned)
- Compression middleware enabled
- Lazy loading of data

### Frontend
- Code splitting by route
- Image optimization
- Lazy loading components
- Service workers (PWA, planned)

### Core Library
- SIMD optimizations for FFT
- Cache-friendly data structures
- Minimal allocations in hot paths

## Scalability

### Horizontal Scaling
- Stateless REST API
- WebSocket load balancing via Socket.io
- Distributed database (future)

### Vertical Scaling
- Database query optimization
- Caching layer (Redis, planned)
- Background job processing (Bull, planned)

## Future Enhancements

1. **Analytics & Reporting**
   - Dashboard with trends
   - PDF report generation
   - Email notifications

2. **Advanced Features**
   - Machine learning for anomaly detection
   - Predictive maintenance
   - Comparative analysis tools

3. **Integration**
   - Integration with manufacturing systems
   - API documentation (Swagger/OpenAPI)
   - Webhook support

4. **Platforms**
   - macOS support
   - iOS app
   - Linux ARM (Raspberry Pi)

## Testing Strategy

### Unit Tests
- Core library: Google Test (C++)
- Backend: Jest
- Frontend: React Testing Library + Vitest

### Integration Tests
- API endpoint tests
- Database operations
- WebSocket communication

### E2E Tests
- User workflow testing
- Multi-platform testing
- Performance testing

## Monitoring & Logging

### Backend Logging
- Winston logger (planned upgrade from console)
- Error tracking (Sentry integration)
- Performance monitoring (New Relic, planned)

### Frontend Logging
- Browser console
- Error reporting
- Analytics events

## References

- [Backend README](./backend/README.md)
- [Web App README](./web/README.md)
- [Core Library README](./lib/coil_analyzer_core/README.md)
- [API Documentation](./API.md)

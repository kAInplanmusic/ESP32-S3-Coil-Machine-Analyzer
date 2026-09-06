# Coil Machine Analyzer Backend

Cross-platform REST API and WebSocket server for real-time audio analysis.

## Features

- **REST API** for measurement management
- **WebSocket** support for real-time analysis streaming
- **SQLite** database for measurement persistence
- **Data Export** in CSV and JSON formats
- **CORS** support for web frontend

## Getting Started

### Installation

```bash
npm install
```

### Configuration

Copy `.env.example` to `.env` and adjust as needed:

```bash
cp .env.example .env
```

### Running

#### Development

```bash
npm run dev
```

#### Production

```bash
npm start
```

## API Endpoints

### Measurements

- `GET /api/v1/measurements` - List all measurements
- `GET /api/v1/measurements/:id` - Get single measurement
- `POST /api/v1/measurements` - Create new measurement
- `DELETE /api/v1/measurements/:id` - Delete measurement

### Analysis

- `GET /api/v1/analysis/:measurementId` - Get analysis results
- `POST /api/v1/analysis/stream` - Real-time analysis

### Export

- `GET /api/v1/export/:id/csv` - Export as CSV
- `GET /api/v1/export/:id/json` - Export as JSON

## WebSocket

Connect to `ws://localhost:3001/ws/measurements` for real-time updates.

### Message Types

- `MEASUREMENT_START` - Begin measurement
- `AUDIO_DATA` - Send audio samples
- `MEASUREMENT_END` - Complete measurement
- `PING` - Health check (receive `PONG`)

## Database Schema

### measurements
- id (TEXT, PRIMARY KEY)
- created_at (DATETIME)
- updated_at (DATETIME)
- cps (REAL)
- quality_score (INTEGER)
- duration_ms (INTEGER)
- sample_count (INTEGER)
- data_json (TEXT)
- exported (BOOLEAN)

### waveforms
- id (TEXT, PRIMARY KEY)
- measurement_id (TEXT, FOREIGN KEY)
- cycle_number (INTEGER)
- timestamp_ms (INTEGER)
- points_json (TEXT)
- harmonic_ratio_h2 (REAL)
- harmonic_ratio_h3 (REAL)
- decay_rate (REAL)

### sessions
- id (TEXT, PRIMARY KEY)
- created_at (DATETIME)
- device_info_json (TEXT)
- calibration_data_json (TEXT)
- measurement_ids (TEXT)

## Development

### Testing

```bash
npm test
```

### Linting

```bash
npm run lint
```

## License

MIT

import express from 'express';
import cors from 'cors';
import helmet from 'helmet';
import compression from 'compression';
import dotenv from 'dotenv';
import { WebSocketServer } from 'ws';
import { createServer } from 'http';
import path from 'path';
import { fileURLToPath } from 'url';

import { initializeDatabase } from './database/db.js';
import { setupRoutes } from './routes/index.js';
import { setupWebSocket } from './websocket/ws-handler.js';
import { errorHandler } from './middleware/errorHandler.js';
import { logger } from './utils/logger.js';

dotenv.config();

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const PORT = process.env.PORT || 3001;
const WS_PORT = process.env.WS_PORT || 3002;

// Express app
const app = express();
const server = createServer(app);

// Middleware
app.use(helmet());
app.use(compression());
app.use(cors({
  origin: (process.env.ALLOWED_ORIGINS || 'http://localhost:3000').split(','),
  credentials: true
}));
app.use(express.json({ limit: process.env.MAX_FILE_UPLOAD_SIZE || '50mb' }));
app.use(express.urlencoded({ limit: process.env.MAX_FILE_UPLOAD_SIZE || '50mb' }));

// Health check
app.get('/health', (req, res) => {
  res.json({ status: 'ok', version: '3.0.0', timestamp: new Date().toISOString() });
});

// API routes
app.use('/api/v1', setupRoutes());

// Static files (for documentation)
app.use('/docs', express.static(path.join(__dirname, '../docs')));

// Error handling
app.use(errorHandler);

// WebSocket Server
const wss = new WebSocketServer({ noServer: true });
setupWebSocket(wss);

server.on('upgrade', (request, socket, head) => {
  if (request.url === '/ws/measurements') {
    wss.handleUpgrade(request, socket, head, (ws) => {
      wss.emit('connection', ws, request);
    });
  } else {
    socket.destroy();
  }
});

// Start server
async function start() {
  try {
    // Initialize database
    await initializeDatabase();
    logger.info('Database initialized');

    // Listen
    server.listen(PORT, () => {
      logger.info(`🚀 Coil Analyzer Backend running on port ${PORT}`);
      logger.info(`📊 API: http://localhost:${PORT}/api/v1`);
      logger.info(`📡 WebSocket: ws://localhost:${PORT}/ws/measurements`);
      logger.info(`📖 Docs: http://localhost:${PORT}/docs`);
    });
  } catch (error) {
    logger.error('Failed to start server:', error);
    process.exit(1);
  }
}

start();

export default app;

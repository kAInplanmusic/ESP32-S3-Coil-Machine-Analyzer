import fs from 'fs';
import path from 'path';
import { fileURLToPath } from 'url';

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const LOG_LEVEL = process.env.LOG_LEVEL || 'info';
const LOG_DIR = path.join(__dirname, '../../logs');

// Create logs directory if it doesn't exist
if (!fs.existsSync(LOG_DIR)) {
  fs.mkdirSync(LOG_DIR, { recursive: true });
}

const LOG_LEVELS = {
  error: 0,
  warn: 1,
  info: 2,
  debug: 3,
};

const CURRENT_LEVEL = LOG_LEVELS[LOG_LEVEL] || LOG_LEVELS.info;

function formatTimestamp() {
  return new Date().toISOString();
}

function log(level, message, data = null) {
  if (LOG_LEVELS[level] > CURRENT_LEVEL) return;

  const timestamp = formatTimestamp();
  const logEntry = {
    timestamp,
    level: level.toUpperCase(),
    message,
    ...(data && { data }),
  };

  const logString = `[${logEntry.timestamp}] ${logEntry.level}: ${logEntry.message}${
    data ? ` ${JSON.stringify(data)}` : ''
  }`;

  // Console output with colors
  const colors = {
    error: '\x1b[31m', // Red
    warn: '\x1b[33m',  // Yellow
    info: '\x1b[36m',  // Cyan
    debug: '\x1b[35m', // Magenta
    reset: '\x1b[0m',
  };

  console.log(`${colors[level] || colors.reset}${logString}${colors.reset}`);

  // File logging
  const logFile = path.join(LOG_DIR, `${level}.log`);
  fs.appendFileSync(logFile, logString + '\n');
}

export const logger = {
  error: (message, data) => log('error', message, data),
  warn: (message, data) => log('warn', message, data),
  info: (message, data) => log('info', message, data),
  debug: (message, data) => log('debug', message, data),
};

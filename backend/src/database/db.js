import sqlite3 from 'sqlite3';
import { promises as fs } from 'fs';
import path from 'path';
import { fileURLToPath } from 'url';
import { logger } from '../utils/logger.js';

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const DB_PATH = process.env.DATABASE_PATH || path.join(__dirname, '../../data/measurements.db');

// Ensure data directory exists
const DATA_DIR = path.dirname(DB_PATH);
await fs.mkdir(DATA_DIR, { recursive: true }).catch(() => {});

let db = null;

export async function initializeDatabase() {
  return new Promise((resolve, reject) => {
    db = new sqlite3.Database(DB_PATH, (err) => {
      if (err) {
        logger.error('Database connection error:', err);
        reject(err);
      } else {
        logger.info('Connected to SQLite database');
        createTables()
          .then(resolve)
          .catch(reject);
      }
    });
  });
}

async function createTables() {
  return new Promise((resolve, reject) => {
    db.serialize(() => {
      // Measurements table
      db.run(`
        CREATE TABLE IF NOT EXISTS measurements (
          id TEXT PRIMARY KEY,
          created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
          updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
          cps REAL,
          quality_score INTEGER,
          duration_ms INTEGER,
          sample_count INTEGER,
          data_json TEXT,
          exported BOOLEAN DEFAULT 0
        )
      `, (err) => {
        if (err) reject(err);
      });

      // Waveforms table
      db.run(`
        CREATE TABLE IF NOT EXISTS waveforms (
          id TEXT PRIMARY KEY,
          measurement_id TEXT,
          cycle_number INTEGER,
          timestamp_ms INTEGER,
          points_json TEXT,
          harmonic_ratio_h2 REAL,
          harmonic_ratio_h3 REAL,
          decay_rate REAL,
          FOREIGN KEY(measurement_id) REFERENCES measurements(id)
        )
      `, (err) => {
        if (err) reject(err);
      });

      // Sessions table
      db.run(`
        CREATE TABLE IF NOT EXISTS sessions (
          id TEXT PRIMARY KEY,
          created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
          device_info_json TEXT,
          calibration_data_json TEXT,
          measurement_ids TEXT
        )
      `, (err) => {
        if (err) reject(err);
        resolve();
      });
    });
  });
}

export function getDatabase() {
  return db;
}

export function dbRun(sql, params = []) {
  return new Promise((resolve, reject) => {
    db.run(sql, params, function(err) {
      if (err) reject(err);
      else resolve({ lastID: this.lastID, changes: this.changes });
    });
  });
}

export function dbGet(sql, params = []) {
  return new Promise((resolve, reject) => {
    db.get(sql, params, (err, row) => {
      if (err) reject(err);
      else resolve(row);
    });
  });
}

export function dbAll(sql, params = []) {
  return new Promise((resolve, reject) => {
    db.all(sql, params, (err, rows) => {
      if (err) reject(err);
      else resolve(rows || []);
    });
  });
}

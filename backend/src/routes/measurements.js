import express from 'express';
import { v4 as uuidv4 } from 'uuid';
import { dbRun, dbGet, dbAll } from '../database/db.js';
import { logger } from '../utils/logger.js';

export function measurementRoutes() {
  const router = express.Router();

  // GET all measurements
  router.get('/', async (req, res, next) => {
    try {
      const measurements = await dbAll(
        'SELECT id, created_at, cps, quality_score, duration_ms FROM measurements ORDER BY created_at DESC'
      );
      res.json(measurements);
    } catch (error) {
      next(error);
    }
  });

  // GET single measurement
  router.get('/:id', async (req, res, next) => {
    try {
      const measurement = await dbGet(
        'SELECT * FROM measurements WHERE id = ?',
        [req.params.id]
      );
      if (!measurement) {
        return res.status(404).json({ error: 'Measurement not found' });
      }
      
      const waveforms = await dbAll(
        'SELECT * FROM waveforms WHERE measurement_id = ? ORDER BY cycle_number',
        [req.params.id]
      );
      
      res.json({
        ...measurement,
        data: JSON.parse(measurement.data_json || '{}'),
        waveforms: waveforms.map(w => ({
          ...w,
          points: JSON.parse(w.points_json || '[]'),
        })),
      });
    } catch (error) {
      next(error);
    }
  });

  // POST new measurement (receive from frontend)
  router.post('/', async (req, res, next) => {
    try {
      const id = uuidv4();
      const {
        cps,
        qualityScore,
        durationMs,
        sampleCount,
        data,
        waveforms,
      } = req.body;

      // Insert measurement
      await dbRun(
        `INSERT INTO measurements (id, cps, quality_score, duration_ms, sample_count, data_json)
         VALUES (?, ?, ?, ?, ?, ?)`,
        [id, cps, qualityScore, durationMs, sampleCount, JSON.stringify(data || {})]
      );

      // Insert waveforms
      if (waveforms && Array.isArray(waveforms)) {
        for (const wf of waveforms) {
          await dbRun(
            `INSERT INTO waveforms (id, measurement_id, cycle_number, timestamp_ms, points_json, harmonic_ratio_h2, harmonic_ratio_h3, decay_rate)
             VALUES (?, ?, ?, ?, ?, ?, ?, ?)`,
            [
              uuidv4(),
              id,
              wf.cycleNumber,
              wf.timestampMs,
              JSON.stringify(wf.points || []),
              wf.harmonicRatioH2,
              wf.harmonicRatioH3,
              wf.decayRate,
            ]
          );
        }
      }

      logger.info('New measurement created', { id, cps, quality_score: qualityScore });

      res.status(201).json({
        id,
        created_at: new Date().toISOString(),
        cps,
        quality_score: qualityScore,
      });
    } catch (error) {
      next(error);
    }
  });

  // DELETE measurement
  router.delete('/:id', async (req, res, next) => {
    try {
      await dbRun('DELETE FROM waveforms WHERE measurement_id = ?', [req.params.id]);
      await dbRun('DELETE FROM measurements WHERE id = ?', [req.params.id]);
      res.json({ success: true, id: req.params.id });
    } catch (error) {
      next(error);
    }
  });

  return router;
}

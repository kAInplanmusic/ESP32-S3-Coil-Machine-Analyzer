import React, { useState } from 'react';
import {
  StyleSheet,
  View,
  Text,
  TouchableOpacity,
  ScrollView,
  ActivityIndicator,
} from 'react-native';

export default function MeasurementScreen({ navigation }) {
  const [isRecording, setIsRecording] = useState(false);
  const [progress, setProgress] = useState(0);
  const [results, setResults] = useState(null);

  const handleStartMeasurement = async () => {
    setIsRecording(true);
    setProgress(0);
    setResults(null);

    // Simulate measurement progress
    const interval = setInterval(() => {
      setProgress((prev) => {
        if (prev >= 100) {
          clearInterval(interval);
          return 100;
        }
        return prev + Math.random() * 10;
      });
    }, 200);

    // Simulate completion
    setTimeout(() => {
      setIsRecording(false);
      setResults({
        cps: 85.3,
        qualityScore: 92,
        durationMs: 15000,
        harmonicRatioH2: 0.45,
        harmonicRatioH3: 0.12,
      });
    }, 5000);
  };

  return (
    <ScrollView style={styles.container}>
      <View style={styles.header}>
        <Text style={styles.title}>🎯 Live Measurement</Text>
      </View>

      <View style={styles.controls}>
        <TouchableOpacity
          style={[styles.button, isRecording && styles.buttonDisabled]}
          onPress={handleStartMeasurement}
          disabled={isRecording}
        >
          <Text style={styles.buttonText}>
            {isRecording ? '🔴 Recording...' : '▶️ Start Measurement'}
          </Text>
        </TouchableOpacity>
      </View>

      {isRecording && (
        <View style={styles.progressSection}>
          <View style={styles.progressBar}>
            <View
              style={[
                styles.progressFill,
                { width: `${progress}%` },
              ]}
            />
          </View>
          <Text style={styles.progressText}>{Math.round(progress)}%</Text>
        </View>
      )}

      {results && (
        <View style={styles.resultsSection}>
          <Text style={styles.resultsTitle}>Measurement Results</Text>

          <View style={styles.resultsGrid}>
            <View style={styles.resultCard}>
              <Text style={styles.resultLabel}>CPS</Text>
              <Text style={styles.resultValue}>
                {results.cps.toFixed(1)}
              </Text>
            </View>

            <View style={styles.resultCard}>
              <Text style={styles.resultLabel}>Quality</Text>
              <Text style={[styles.resultValue, { color: '#4caf50' }]}>
                {results.qualityScore}%
              </Text>
            </View>

            <View style={styles.resultCard}>
              <Text style={styles.resultLabel}>H2/H1</Text>
              <Text style={styles.resultValue}>
                {results.harmonicRatioH2.toFixed(2)}
              </Text>
            </View>

            <View style={styles.resultCard}>
              <Text style={styles.resultLabel}>H3/H1</Text>
              <Text style={styles.resultValue}>
                {results.harmonicRatioH3.toFixed(2)}
              </Text>
            </View>
          </View>

          <TouchableOpacity
            style={styles.analyzeButton}
            onPress={() =>
              navigation.navigate('Analysis', { measurementId: 'latest' })
            }
          >
            <Text style={styles.analyzeButtonText}>View Detailed Analysis</Text>
          </TouchableOpacity>
        </View>
      )}
    </ScrollView>
  );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    backgroundColor: '#f5f5f5',
  },
  header: {
    paddingVertical: 20,
    paddingHorizontal: 16,
    backgroundColor: '#2196f3',
  },
  title: {
    fontSize: 24,
    fontWeight: 'bold',
    color: '#fff',
    textAlign: 'center',
  },
  controls: {
    padding: 16,
  },
  button: {
    backgroundColor: '#2196f3',
    padding: 16,
    borderRadius: 8,
    alignItems: 'center',
  },
  buttonDisabled: {
    opacity: 0.6,
  },
  buttonText: {
    color: '#fff',
    fontSize: 16,
    fontWeight: '600',
  },
  progressSection: {
    padding: 16,
  },
  progressBar: {
    backgroundColor: '#e0e0e0',
    height: 8,
    borderRadius: 4,
    overflow: 'hidden',
    marginBottom: 8,
  },
  progressFill: {
    backgroundColor: '#4caf50',
    height: '100%',
  },
  progressText: {
    textAlign: 'center',
    fontSize: 14,
    color: '#666',
  },
  resultsSection: {
    padding: 16,
  },
  resultsTitle: {
    fontSize: 18,
    fontWeight: 'bold',
    color: '#333',
    marginBottom: 12,
  },
  resultsGrid: {
    display: 'flex',
    flexDirection: 'column',
  },
  resultCard: {
    backgroundColor: '#fff',
    borderRadius: 8,
    padding: 12,
    marginBottom: 8,
    borderLeftWidth: 4,
    borderLeftColor: '#2196f3',
  },
  resultLabel: {
    fontSize: 12,
    color: '#999',
    marginBottom: 4,
  },
  resultValue: {
    fontSize: 20,
    fontWeight: 'bold',
    color: '#2196f3',
  },
  analyzeButton: {
    backgroundColor: '#4caf50',
    padding: 16,
    borderRadius: 8,
    alignItems: 'center',
    marginTop: 16,
  },
  analyzeButtonText: {
    color: '#fff',
    fontSize: 16,
    fontWeight: '600',
  },
});

import React from 'react';
import { StyleSheet, View, Text, ScrollView } from 'react-native';

export default function AnalysisScreen({ route }) {
  const { measurementId } = route.params || {};

  // Mock data
  const measurement = {
    id: measurementId,
    cps: 85.3,
    qualityScore: 92,
    durationMs: 15000,
    harmonicRatioH2: 0.45,
    harmonicRatioH3: 0.12,
    thd: 48.5,
    decayRate: 0.0234,
    qFactor: 134.5,
  };

  return (
    <ScrollView style={styles.container}>
      <View style={styles.header}>
        <Text style={styles.title}>📈 Detailed Analysis</Text>
      </View>

      <View style={styles.section}>
        <Text style={styles.sectionTitle}>Measurement Parameters</Text>
        <View style={styles.row}>
          <Text style={styles.label}>CPS (Cycles/sec):</Text>
          <Text style={styles.value}>{measurement.cps.toFixed(2)}</Text>
        </View>
        <View style={styles.row}>
          <Text style={styles.label}>Quality Score:</Text>
          <Text style={styles.value}>{measurement.qualityScore}%</Text>
        </View>
        <View style={styles.row}>
          <Text style={styles.label}>Duration:</Text>
          <Text style={styles.value}>{measurement.durationMs}ms</Text>
        </View>
      </View>

      <View style={styles.section}>
        <Text style={styles.sectionTitle}>Harmonic Analysis</Text>
        <View style={styles.row}>
          <Text style={styles.label}>H2/H1 Ratio:</Text>
          <Text style={styles.value}>
            {measurement.harmonicRatioH2.toFixed(3)}
          </Text>
        </View>
        <View style={styles.row}>
          <Text style={styles.label}>H3/H1 Ratio:</Text>
          <Text style={styles.value}>
            {measurement.harmonicRatioH3.toFixed(3)}
          </Text>
        </View>
        <View style={styles.row}>
          <Text style={styles.label}>THD:</Text>
          <Text style={styles.value}>{measurement.thd.toFixed(1)}%</Text>
        </View>
      </View>

      <View style={styles.section}>
        <Text style={styles.sectionTitle}>Decay Analysis</Text>
        <View style={styles.row}>
          <Text style={styles.label}>Decay Rate:</Text>
          <Text style={styles.value}>{measurement.decayRate.toFixed(4)}</Text>
        </View>
        <View style={styles.row}>
          <Text style={styles.label}>Q-Factor:</Text>
          <Text style={styles.value}>{measurement.qFactor.toFixed(2)}</Text>
        </View>
      </View>
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
  section: {
    backgroundColor: '#fff',
    marginHorizontal: 16,
    marginVertical: 12,
    borderRadius: 8,
    padding: 16,
  },
  sectionTitle: {
    fontSize: 16,
    fontWeight: 'bold',
    color: '#2196f3',
    marginBottom: 12,
  },
  row: {
    flexDirection: 'row',
    justifyContent: 'space-between',
    alignItems: 'center',
    paddingVertical: 8,
    borderBottomWidth: 1,
    borderBottomColor: '#f0f0f0',
  },
  label: {
    fontSize: 14,
    color: '#666',
    fontWeight: '600',
    flex: 1,
  },
  value: {
    fontSize: 16,
    color: '#2196f3',
    fontWeight: 'bold',
    textAlign: 'right',
  },
});

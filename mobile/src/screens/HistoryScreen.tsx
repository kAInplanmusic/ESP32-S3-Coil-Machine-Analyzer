import React, { useState, useEffect } from 'react';
import {
  StyleSheet,
  View,
  Text,
  FlatList,
  TouchableOpacity,
  ActivityIndicator,
} from 'react-native';
import AsyncStorage from '@react-native-async-storage/async-storage';

export default function HistoryScreen({ navigation }) {
  const [measurements, setMeasurements] = useState([]);
  const [loading, setLoading] = useState(true);

  useEffect(() => {
    loadMeasurements();
  }, []);

  const loadMeasurements = async () => {
    try {
      // TODO: Load from API
      const saved = await AsyncStorage.getItem('measurements');
      if (saved) {
        setMeasurements(JSON.parse(saved));
      }
    } catch (error) {
      console.error('Failed to load measurements:', error);
    } finally {
      setLoading(false);
    }
  };

  if (loading) {
    return (
      <View style={styles.container}>
        <ActivityIndicator size="large" color="#2196f3" />
      </View>
    );
  }

  const renderItem = ({ item }) => (
    <TouchableOpacity
      style={styles.item}
      onPress={() =>
        navigation.navigate('Analysis', { measurementId: item.id })
      }
    >
      <View style={styles.itemContent}>
        <Text style={styles.itemDate}>
          {new Date(item.createdAt).toLocaleString()}
        </Text>
        <Text style={styles.itemCPS}>CPS: {item.cps?.toFixed(1)}</Text>
        <View
          style={[
            styles.qualityBadge,
            {
              backgroundColor:
                item.qualityScore > 80 ? '#c8e6c9' : '#fff9c4',
            },
          ]}
        >
          <Text
            style={[
              styles.qualityText,
              {
                color: item.qualityScore > 80 ? '#2e7d32' : '#f57f17',
              },
            ]}
          >
            {item.qualityScore}%
          </Text>
        </View>
      </View>
    </TouchableOpacity>
  );

  return (
    <View style={styles.container}>
      <View style={styles.header}>
        <Text style={styles.title}>📊 Measurement History</Text>
      </View>

      {measurements.length === 0 ? (
        <View style={styles.emptyContainer}>
          <Text style={styles.emptyText}>No measurements yet</Text>
          <Text style={styles.emptySubtext}>
            Start a measurement to see results here
          </Text>
        </View>
      ) : (
        <FlatList
          data={measurements}
          renderItem={renderItem}
          keyExtractor={(item) => item.id}
          contentContainerStyle={styles.listContent}
        />
      )}
    </View>
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
  listContent: {
    padding: 16,
  },
  item: {
    backgroundColor: '#fff',
    borderRadius: 8,
    marginBottom: 12,
    shadowColor: '#000',
    shadowOffset: { width: 0, height: 2 },
    shadowOpacity: 0.1,
    shadowRadius: 4,
    elevation: 3,
  },
  itemContent: {
    flexDirection: 'row',
    padding: 12,
    alignItems: 'center',
    justifyContent: 'space-between',
  },
  itemDate: {
    flex: 1,
    fontSize: 12,
    color: '#666',
  },
  itemCPS: {
    fontSize: 14,
    fontWeight: '600',
    color: '#333',
    marginRight: 12,
  },
  qualityBadge: {
    paddingHorizontal: 8,
    paddingVertical: 4,
    borderRadius: 4,
  },
  qualityText: {
    fontSize: 12,
    fontWeight: '600',
  },
  emptyContainer: {
    flex: 1,
    justifyContent: 'center',
    alignItems: 'center',
  },
  emptyText: {
    fontSize: 18,
    color: '#666',
    marginBottom: 8,
  },
  emptySubtext: {
    fontSize: 14,
    color: '#999',
  },
});

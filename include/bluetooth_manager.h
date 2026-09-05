#ifndef BLUETOOTH_MANAGER_H
#define BLUETOOTH_MANAGER_H

#include <Arduino.h>
#include <ArduinoBLE.h>
#include "config.h"
#include "advanced_impact_analyzer.h"

/**
 * @class BluetoothManager
 * @brief BLE (Bluetooth Low Energy) interface for remote data access
 * 
 * Features:
 * - Real-time measurement streaming
 * - Remote parameter adjustment
 * - Data export via BLE
 * - Mobile app compatibility
 */
class BluetoothManager {
public:
    BluetoothManager();
    ~BluetoothManager();
    
    /**
     * Initialize Bluetooth LE
     * @param device_name Name for this BLE device
     * @return true if successful
     */
    bool init(const char* device_name = "CoilAnalyzer");
    
    /**
     * Start advertising
     */
    void startAdvertising();
    
    /**
     * Stop advertising
     */
    void stopAdvertising();
    
    /**
     * Update BLE (handle connections, send data)
     */
    void update();
    
    /**
     * Check if device is connected
     */
    bool isConnected() const { return _connected; }
    
    /**
     * Send measurement data via BLE
     * @param analyzer Advanced analyzer with data
     */
    void sendMeasurementData(const AdvancedImpactAnalyzer& analyzer);
    
    /**
     * Send real-time CPS update
     * @param cps Current CPS value
     * @param quality Quality score
     */
    void sendRealtimeCPS(float cps, uint8_t quality);
    
    /**
     * Send measurement point data
     * @param point Measurement point
     * @param point_number Which point (1-N)
     */
    void sendMeasurementPoint(const AdvancedImpactAnalyzer::MeasurementPoint& point,
                              uint8_t point_number);
    
    /**
     * Send statistics
     * @param stats Statistics data
     */
    void sendStatistics(const AdvancedImpactAnalyzer::AdvancedStats& stats);
    
private:
    BLEService* _service;
    BLECharacteristic* _cps_characteristic;        // Read/Notify CPS
    BLECharacteristic* _quality_characteristic;    // Read/Notify Quality
    BLECharacteristic* _point_characteristic;      // Read/Notify Point data
    BLECharacteristic* _command_characteristic;    // Write for remote commands
    
    bool _initialized;
    bool _connected;
    unsigned long _last_update;
    
    static const char* CHARACTERISTIC_UUID_CPS;
    static const char* CHARACTERISTIC_UUID_QUALITY;
    static const char* CHARACTERISTIC_UUID_POINT;
    static const char* CHARACTERISTIC_UUID_CMD;
    static const char* SERVICE_UUID;
    
    /**
     * Setup BLE service and characteristics
     */
    void setupBLEService();
    
    /**
     * Handle incoming BLE commands
     */
    void handleBLECommands();
};

#endif // BLUETOOTH_MANAGER_H

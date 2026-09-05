#include <Arduino.h>
#include "config.h"
#include "i2s_audio.h"
#include "signal_processor.h"
#include "display_manager.h"

static const char* TAG = "CoilAnalyzer";

// Global objects
I2SAudio audio;
SignalProcessor processor;
DisplayManager display;

// Task handles
TaskHandle_t audioTaskHandle = nullptr;
TaskHandle_t processingTaskHandle = nullptr;
TaskHandle_t displayTaskHandle = nullptr;

// Ring buffer for audio samples
static const size_t SAMPLE_BUFFER_SIZE = 4096;
int16_t sampleBuffer[SAMPLE_BUFFER_SIZE];

// Measurement state
struct MeasurementState {
    float currentCPS = 0.0f;
    uint8_t qualityScore = 0;
    bool isRecording = false;
    unsigned long recordingStartTime = 0;
} measurementState;

// Task: Audio capture from I2S
void audioCaptureTask(void* pvParameters) {
    while (1) {
        size_t samples_read = audio.readSamples(sampleBuffer, SAMPLE_BUFFER_SIZE);
        
        if (samples_read > 0) {
            // Process samples
            processor.processSamples(sampleBuffer, samples_read);
            
            // Compute FFT every FFT_SIZE samples
            static size_t accumulated = 0;
            accumulated += samples_read;
            
            if (accumulated >= FFT_SIZE) {
                processor.computeFFT();
                processor.detectPeaks();
                accumulated = 0;
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

// Task: Measurement and CPS calculation
void measurementTask(void* pvParameters) {
    while (1) {
        // Calculate CPS
        measurementState.currentCPS = processor.calculateCPS(2.0f);
        
        // Update quality score
        processor.updateQualityScore();
        measurementState.qualityScore = processor.getQualityScore();
        
        // Log to serial
        if (LOG_SERIAL) {
            Serial.printf("CPS: %.2f (%.0f%%) | Quality: %d%%\n",
                         measurementState.currentCPS,
                         (measurementState.currentCPS / CPS_MAX) * 100.0f,
                         measurementState.qualityScore);
        }
        
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

// Task: Display update
void displayUpdateTask(void* pvParameters) {
    while (1) {
        display.update(processor, measurementState.currentCPS, 
                      measurementState.qualityScore);
        
        vTaskDelay(pdMS_TO_TICKS(UI_UPDATE_INTERVAL));
    }
}

void setup() {
    // Initialize serial communication
    Serial.begin(115200);
    delay(1000);  // Wait for serial to be ready
    
    Serial.println("\n\n=====================================");
    Serial.println("ESP32-S3 Coil Machine Analyzer");
    Serial.println("=====================================");
    Serial.printf("Build: %s %s\n", __DATE__, __TIME__);
    Serial.printf("Sample Rate: %d Hz\n", SAMPLE_RATE);
    Serial.printf("FFT Size: %d\n", FFT_SIZE);
    Serial.printf("CPS Range: %.2f - %.2f\n", CPS_MIN, CPS_MAX);
    Serial.println();
    
    // Initialize display
    Serial.println("[INIT] Initializing display...");
    if (!display.init()) {
        Serial.println("[ERROR] Display initialization failed!");
        display.drawError("Init Error", "Display failed!");
        while (1) {
            delay(1000);
        }
    }
    display.drawInitScreen();
    Serial.println("[OK] Display initialized");
    
    // Initialize audio
    Serial.println("[INIT] Initializing audio input...");
    if (!audio.init() || !audio.start()) {
        Serial.println("[ERROR] Audio initialization failed!");
        display.drawError("Audio Error", "Microphone init failed!");
        while (1) {
            delay(1000);
        }
    }
    Serial.println("[OK] Audio input initialized");
    
    // Calibrate microphone
    Serial.println("[INIT] Calibrating microphone...");
    delay(1000);  // Let microphone settle
    audio.calibrateReference();
    Serial.printf("[OK] Calibration complete. Gain: %.2f dB\n", audio.getGain());
    
    // Initialize signal processor
    Serial.println("[INIT] Initializing signal processor...");
    if (!processor.init()) {
        Serial.println("[ERROR] Signal processor initialization failed!");
        display.drawError("Processor Error", "Signal init failed!");
        while (1) {
            delay(1000);
        }
    }
    Serial.println("[OK] Signal processor initialized");
    
    // Create FreeRTOS tasks
    Serial.println("[INIT] Creating FreeRTOS tasks...");
    
    xTaskCreatePinnedToCore(
        audioCaptureTask,      // Task function
        "AudioCapture",        // Task name
        4096,                  // Stack size
        nullptr,               // Parameters
        3,                     // Priority (high)
        &audioTaskHandle,      // Task handle
        0                      // Core 0
    );
    
    xTaskCreatePinnedToCore(
        measurementTask,
        "Measurement",
        2048,
        nullptr,
        2,
        &processingTaskHandle,
        1                      // Core 1
    );
    
    xTaskCreatePinnedToCore(
        displayUpdateTask,
        "DisplayUpdate",
        2048,
        nullptr,
        1,
        &displayTaskHandle,
        1
    );
    
    Serial.println("[OK] All tasks created");
    Serial.println("=====================================");
    Serial.println("System ready! Starting measurement...");
    Serial.println();
    
    measurementState.isRecording = true;
}

void loop() {
    // Main loop runs at lower priority
    // Most work is done in FreeRTOS tasks
    
    // Check for serial commands
    if (Serial.available()) {
        char cmd = Serial.read();
        
        switch (cmd) {
            case 'r':  // Reset
                Serial.println("[CMD] Resetting impact history...");
                processor.clearImpactEvents();
                break;
                
            case 'v':  // Change view
                {
                    DisplayManager::ViewMode current = display.getViewMode();
                    DisplayManager::ViewMode next = (DisplayManager::ViewMode)((current + 1) % 4);
                    display.setViewMode(next);
                    Serial.printf("[CMD] View changed to mode %d\n", next);
                }
                break;
                
            case 's':  // Print statistics
                {
                    auto stats = processor.getImpactIntervalStats();
                    Serial.println("\n[STATS] Impact Intervals:");
                    Serial.printf("  Mean: %.2f ms\n", stats[0]);
                    Serial.printf("  StdDev: %.2f ms\n", stats[1]);
                    Serial.printf("  Min: %.2f ms\n", stats[2]);
                    Serial.printf("  Max: %.2f ms\n", stats[3]);
                    Serial.printf("  Impacts detected: %d\n", processor.getImpactEvents().size());
                }
                break;
                
            case 'c':  // Calibrate
                Serial.println("[CMD] Recalibrating microphone...");
                audio.calibrateReference();
                break;
                
            case '+':  // Increase gain
                {
                    float gain = audio.getGain() + 5.0f;
                    audio.setGain(gain);
                }
                break;
                
            case '-':  // Decrease gain
                {
                    float gain = audio.getGain() - 5.0f;
                    audio.setGain(gain);
                }
                break;
                
            case 'h':  // Help
                Serial.println("\n[HELP] Serial Commands:");
                Serial.println("  r - Reset impact history");
                Serial.println("  v - Change view mode");
                Serial.println("  s - Print statistics");
                Serial.println("  c - Calibrate microphone");
                Serial.println("  + - Increase gain (+5dB)");
                Serial.println("  - - Decrease gain (-5dB)");
                Serial.println("  h - Show this help");
                Serial.println();
                break;
                
            default:
                break;
        }
    }
    
    delay(100);
}

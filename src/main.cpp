#include <Arduino.h>
#include "config.h"
#include "i2s_audio.h"
#include "signal_processor.h"
#include "display_manager.h"
#include "impact_analyzer.h"
#include "measurement_ui.h"

static const char* TAG = "CoilAnalyzer";

// Global objects
I2SAudio audio;
SignalProcessor processor;
DisplayManager display;
ImpactAnalyzer impactAnalyzer;
MeasurementUI measurementUI;

// Task handles
TaskHandle_t audioTaskHandle = nullptr;
TaskHandle_t processingTaskHandle = nullptr;
TaskHandle_t displayTaskHandle = nullptr;
TaskHandle_t uiTaskHandle = nullptr;

// Ring buffer for audio samples
static const size_t SAMPLE_BUFFER_SIZE = 4096;
int16_t sampleBuffer[SAMPLE_BUFFER_SIZE];
uint32_t sample_index = 0;

// Global measurement state
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
            // Process samples through impact analyzer
            for (size_t i = 0; i < samples_read; i++) {
                // Feed to impact analyzer (3-point analysis)
                impactAnalyzer.processSample(sampleBuffer[i], sample_index + i);
                
                // Also process for general signal analysis
                processor.processSamples(&sampleBuffer[i], 1);
            }
            sample_index += samples_read;
            
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
        // Get current measurement state
        if (impactAnalyzer.isMeasuring()) {
            // Update CPS and quality from impact analyzer
            measurementState.currentCPS = impactAnalyzer.getCPS();
            const auto& stats = impactAnalyzer.getStats();
            measurementState.qualityScore = stats.quality_score;
        } else {
            // Fallback to signal processor calculation
            measurementState.currentCPS = processor.calculateCPS(2.0f);
            processor.updateQualityScore();
            measurementState.qualityScore = processor.getQualityScore();
        }
        
        // Log to serial (debug)
        if (LOG_SERIAL && impactAnalyzer.isMeasuring()) {
            static unsigned long last_log = 0;
            if (millis() - last_log > 1000) {
                Serial.printf("CPS: %.2f | Quality: %d%% | Progress: %d%%\n",
                             measurementState.currentCPS,
                             measurementState.qualityScore,
                             impactAnalyzer.getMeasurementProgress());
                last_log = millis();
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

// Task: Display update
void displayUpdateTask(void* pvParameters) {
    while (1) {
        // Update measurement UI
        measurementUI.update();
        
        vTaskDelay(pdMS_TO_TICKS(UI_UPDATE_INTERVAL));
    }
}

void setup() {
    // Initialize serial communication
    Serial.begin(SERIAL_BAUD_RATE);
    delay(1000);
    
    Serial.println("\n\n=====================================");
    Serial.println("ESP32-S3 Coil Machine Analyzer v2.0");
    Serial.println("3-Point Impact Analysis (0-250 CPS)");
    Serial.println("=====================================");
    Serial.printf("Build: %s %s\n", __DATE__, __TIME__);
    Serial.printf("Sample Rate: %d Hz\n", SAMPLE_RATE);
    Serial.printf("FFT Size: %d\n", FFT_SIZE);
    Serial.printf("CPS Range: %.0f - %.0f\n", CPS_MIN, CPS_MAX);
    Serial.println();
    
    // Initialize display
    Serial.println("[INIT] Initializing display...");
    if (!display.init()) {
        Serial.println("[ERROR] Display initialization failed!");
        while (1) {
            delay(1000);
        }
    }
    display.drawInitScreen();
    Serial.println("[OK] Display initialized");
    delay(500);
    
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
    delay(500);
    
    // Calibrate microphone
    Serial.println("[INIT] Calibrating microphone...");
    delay(1000);
    audio.calibrateReference();
    Serial.printf("[OK] Calibration complete. Gain: %.2f dB\n", audio.getGain());
    delay(500);
    
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
    delay(500);
    
    // Initialize impact analyzer (3-point analysis)
    Serial.println("[INIT] Initializing impact analyzer...");
    if (!impactAnalyzer.init()) {
        Serial.println("[ERROR] Impact analyzer initialization failed!");
        display.drawError("Analyzer Error", "Impact analysis failed!");
        while (1) {
            delay(1000);
        }
    }
    // Set microphone position
    impactAnalyzer.setMicrophonePosition(MICROPHONE_DISTANCE_MM, MICROPHONE_ANGLE_DEG);
    Serial.printf("[OK] Impact analyzer initialized (%.0f mm, %.0f°)\n",
                 MICROPHONE_DISTANCE_MM, MICROPHONE_ANGLE_DEG);
    delay(500);
    
    // Initialize measurement UI
    Serial.println("[INIT] Initializing measurement UI...");
    if (!measurementUI.init(&display, &impactAnalyzer)) {
        Serial.println("[ERROR] UI initialization failed!");
        display.drawError("UI Error", "Measurement UI failed!");
        while (1) {
            delay(1000);
        }
    }
    Serial.println("[OK] Measurement UI initialized");
    delay(500);
    
    // Create FreeRTOS tasks
    Serial.println("[INIT] Creating FreeRTOS tasks...");
    
    xTaskCreatePinnedToCore(
        audioCaptureTask,
        "AudioCapture",
        AUDIO_TASK_STACK_SIZE,
        nullptr,
        3,  // High priority
        &audioTaskHandle,
        0   // Core 0
    );
    
    xTaskCreatePinnedToCore(
        measurementTask,
        "Measurement",
        PROCESSING_TASK_STACK,
        nullptr,
        2,  // Medium priority
        &processingTaskHandle,
        1   // Core 1
    );
    
    xTaskCreatePinnedToCore(
        displayUpdateTask,
        "DisplayUpdate",
        DISPLAY_TASK_STACK_SIZE,
        nullptr,
        1,  // Low priority
        &displayTaskHandle,
        1
    );
    
    Serial.println("[OK] All tasks created");
    Serial.println("=====================================");
    Serial.println("System ready! Waiting for START...");
    Serial.println();
}

void loop() {
    // Main loop - handle serial commands and button input
    
    // Check for serial commands
    if (Serial.available()) {
        char cmd = Serial.read();
        
        switch (cmd) {
            case 's':  // START measurement
                Serial.println("[CMD] START button pressed");
                measurementUI.handleStartButton();
                break;
                
            case 'r':  // Reset
                Serial.println("[CMD] Resetting analyzer...");
                impactAnalyzer.reset();
                measurementUI.setState(MeasurementUI::STATE_IDLE);
                break;
                
            case 'c':  // Calibrate microphone
                Serial.println("[CMD] Recalibrating microphone...");
                audio.calibrateReference();
                break;
                
            case '+':  // Increase gain
                {
                    float gain = audio.getGain() + 5.0f;
                    audio.setGain(gain);
                    Serial.printf("[CMD] Gain increased to %.1f dB\n", gain);
                }
                break;
                
            case '-':  // Decrease gain
                {
                    float gain = audio.getGain() - 5.0f;
                    audio.setGain(gain);
                    Serial.printf("[CMD] Gain decreased to %.1f dB\n", gain);
                }
                break;
                
            case 'd':  // Adjust microphone distance
                {
                    float dist = MICROPHONE_DISTANCE_MM;
                    Serial.printf("[CMD] Current distance: %.0f mm\n", dist);
                    Serial.println("[CMD] Enter new distance (10-500 mm)");
                }
                break;
                
            case 'a':  // Adjust microphone angle
                {
                    float angle = MICROPHONE_ANGLE_DEG;
                    Serial.printf("[CMD] Current angle: %.0f°\n", angle);
                    Serial.println("[CMD] Enter new angle (-180 to 180°)");
                }
                break;
                
            case 'i':  // Print current stats
                {
                    const auto& stats = impactAnalyzer.getStats();
                    Serial.println("\n[STATS] Current Measurement:");
                    Serial.printf("  CPS: %.2f\n", stats.cps_calculated);
                    Serial.printf("  Quality: %d%%\n", stats.quality_score);
                    Serial.printf("  Consistency: %.1f%%\n", stats.consistency_percent);
                    Serial.printf("  Samples: %d\n", stats.samples_collected);
                    Serial.printf("  Mean Period: %.2f ms\n", stats.mean_period_ms);
                    Serial.printf("  Jitter A: %.3f ms\n", stats.jitter_A_ms);
                    Serial.printf("  Jitter B: %.3f ms\n", stats.jitter_B_ms);
                    Serial.printf("  Jitter C: %.3f ms\n", stats.jitter_C_ms);
                    Serial.println();
                }
                break;
                
            case 'h':  // Help
                Serial.println("\n[HELP] Serial Commands:");
                Serial.println("  s - START measurement (100 impacts)");
                Serial.println("  r - Reset analyzer");
                Serial.println("  c - Calibrate microphone");
                Serial.println("  + - Increase gain (+5dB)");
                Serial.println("  - - Decrease gain (-5dB)");
                Serial.println("  d - Adjust microphone distance");
                Serial.println("  a - Adjust microphone angle");
                Serial.println("  i - Print current statistics");
                Serial.println("  h - Show this help");
                Serial.println();
                break;
                
            default:
                break;
        }
    }
    
    delay(100);
}

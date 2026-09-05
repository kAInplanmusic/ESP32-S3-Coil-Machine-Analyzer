#include <Arduino.h>
#include "config.h"
#include "i2s_audio.h"
#include "signal_processor.h"
#include "display_manager.h"
#include "advanced_impact_analyzer.h"
#include "advanced_measurement_ui.h"

static const char* TAG = "CoilAnalyzer_Advanced";

// Global objects
I2SAudio audio;
SignalProcessor processor;
DisplayManager display;
AdvancedImpactAnalyzer advancedAnalyzer;
AdvancedMeasurementUI advancedUI;

// Task handles
TaskHandle_t audioTaskHandle = nullptr;
TaskHandle_t processingTaskHandle = nullptr;
TaskHandle_t displayTaskHandle = nullptr;

// Ring buffer for audio samples
static const size_t SAMPLE_BUFFER_SIZE = 4096;
int16_t sampleBuffer[SAMPLE_BUFFER_SIZE];
uint32_t sample_index = 0;

// Global measurement state
struct SystemState {
    float currentCPS = 0.0f;
    uint8_t qualityScore = 0;
    bool isRecording = false;
    unsigned long recordingStartTime = 0;
} systemState;

/**
 * Task: Audio capture from I2S microphone
 * Priority: HIGH (Core 0)
 */
void audioCaptureTask(void* pvParameters) {
    ESP_LOGI(TAG, "Audio capture task started on Core 0");
    
    while (1) {
        size_t samples_read = audio.readSamples(sampleBuffer, SAMPLE_BUFFER_SIZE);
        
        if (samples_read > 0) {
            // Process samples through advanced analyzer (multi-point waveform analysis)
            for (size_t i = 0; i < samples_read; i++) {
                advancedAnalyzer.processSample(sampleBuffer[i], sample_index + i);
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

/**
 * Task: Measurement processing and statistics calculation
 * Priority: MEDIUM (Core 1)
 */
void measurementTask(void* pvParameters) {
    ESP_LOGI(TAG, "Measurement task started on Core 1");
    
    while (1) {
        // Get current state from advanced analyzer
        if (advancedAnalyzer.isMeasuring()) {
            systemState.currentCPS = advancedAnalyzer.getCPS();
            const auto& stats = advancedAnalyzer.getStats();
            systemState.qualityScore = stats.quality_score;
        } else {
            systemState.currentCPS = processor.calculateCPS(2.0f);
            processor.updateQualityScore();
            systemState.qualityScore = processor.getQualityScore();
        }
        
        // Log to serial (debug)
        if (LOG_SERIAL && advancedAnalyzer.isMeasuring()) {
            static unsigned long last_log = 0;
            if (millis() - last_log > 2000) {
                const auto& stats = advancedAnalyzer.getStats();
                Serial.printf("[MEASURE] CPS: %.2f | Quality: %d%% | Progress: %d%% | Waveforms: %d\n",
                             systemState.currentCPS,
                             systemState.qualityScore,
                             advancedAnalyzer.getMeasurementProgress(),
                             stats.samples_collected);
                
                if (stats.samples_collected > 0) {
                    Serial.printf("          Mean H2/H1: %.2f | Mean H3/H1: %.2f | Decay: %.4f\n",
                                 stats.mean_harmonic_2_1,
                                 stats.mean_harmonic_3_1,
                                 stats.mean_decay_rate);
                }
                last_log = millis();
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

/**
 * Task: Display update
 * Priority: LOW (Core 1)
 */
void displayUpdateTask(void* pvParameters) {
    ESP_LOGI(TAG, "Display update task started on Core 1");
    
    while (1) {
        advancedUI.update();
        vTaskDelay(pdMS_TO_TICKS(UI_UPDATE_INTERVAL));
    }
}

/**
 * Initialize all systems
 */
void setup() {
    // Initialize serial communication
    Serial.begin(SERIAL_BAUD_RATE);
    delay(1000);
    
    Serial.println("\n\n=====================================");
    Serial.println("ESP32-S3 Coil Machine Analyzer v3.0");
    Serial.println("Advanced Multi-Point Waveform Analysis");
    Serial.println("0-250 CPS with Harmonic Analysis");
    Serial.println("=====================================");
    Serial.printf("Build: %s %s\n", __DATE__, __TIME__);
    Serial.printf("Sample Rate: %d Hz\n", SAMPLE_RATE);
    Serial.printf("FFT Size: %d (Resolution: %.2f Hz/bin)\n", FFT_SIZE, FFT_FREQ_RESOLUTION);
    Serial.printf("CPS Range: %.0f - %.0f\n", CPS_MIN, CPS_MAX);
    Serial.printf("Measurement: 100 impact cycles\n");
    Serial.println();
    
    // ========== DISPLAY INITIALIZATION ==========
    Serial.println("[INIT] Initializing display...");
    if (!display.init()) {
        Serial.println("[ERROR] Display initialization failed!");
        while (1) delay(1000);
    }
    display.drawInitScreen();
    Serial.println("[OK] Display initialized (480x320 TFT)");
    delay(500);
    
    // ========== AUDIO INITIALIZATION ==========
    Serial.println("[INIT] Initializing audio input...");
    if (!audio.init() || !audio.start()) {
        Serial.println("[ERROR] Audio initialization failed!");
        display.drawError("Audio Error", "Microphone init failed!");
        while (1) delay(1000);
    }
    Serial.println("[OK] Audio input initialized (I2S, 44.1 kHz, Mono)");
    delay(500);
    
    // ========== AUDIO CALIBRATION ==========
    Serial.println("[INIT] Calibrating microphone...");
    display.drawInitScreen();
    delay(1000);
    audio.calibrateReference();
    Serial.printf("[OK] Calibration complete. Gain: %.2f dB\n", audio.getGain());
    delay(500);
    
    // ========== SIGNAL PROCESSOR INITIALIZATION ==========
    Serial.println("[INIT] Initializing signal processor...");
    if (!processor.init()) {
        Serial.println("[ERROR] Signal processor initialization failed!");
        display.drawError("Processor Error", "Signal init failed!");
        while (1) delay(1000);
    }
    Serial.println("[OK] Signal processor initialized (FFT, Peak detection)");
    delay(500);
    
    // ========== ADVANCED IMPACT ANALYZER INITIALIZATION ==========
    Serial.println("[INIT] Initializing Advanced Impact Analyzer...");
    if (!advancedAnalyzer.init(12)) {  // 12 measurement points max
        Serial.println("[ERROR] Advanced analyzer initialization failed!");
        display.drawError("Analyzer Error", "Waveform analysis failed!");
        while (1) delay(1000);
    }
    // Set microphone position
    advancedAnalyzer.setMicrophonePosition(MICROPHONE_DISTANCE_MM, MICROPHONE_ANGLE_DEG);
    Serial.printf("[OK] Advanced analyzer initialized (multi-point waveform)\n");
    Serial.printf("     Microphone: %.0f mm, %.0f°\n", MICROPHONE_DISTANCE_MM, MICROPHONE_ANGLE_DEG);
    delay(500);
    
    // ========== ADVANCED MEASUREMENT UI INITIALIZATION ==========
    Serial.println("[INIT] Initializing Advanced Measurement UI...");
    if (!advancedUI.init(&display, &advancedAnalyzer)) {
        Serial.println("[ERROR] UI initialization failed!");
        display.drawError("UI Error", "Measurement UI failed!");
        while (1) delay(1000);
    }
    Serial.println("[OK] Advanced Measurement UI initialized");
    Serial.println("     - Waveform visualization");
    Serial.println("     - Measurement point table");
    Serial.println("     - Harmonic analysis");
    Serial.println("     - Decay rate analysis");
    Serial.println("     - Quality breakdown");
    delay(500);
    
    // ========== FREERTOS TASK CREATION ==========
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
    Serial.println("     [OK] Audio capture task (Core 0, priority 3)");
    
    xTaskCreatePinnedToCore(
        measurementTask,
        "Measurement",
        PROCESSING_TASK_STACK,
        nullptr,
        2,  // Medium priority
        &processingTaskHandle,
        1   // Core 1
    );
    Serial.println("     [OK] Measurement task (Core 1, priority 2)");
    
    xTaskCreatePinnedToCore(
        displayUpdateTask,
        "DisplayUpdate",
        DISPLAY_TASK_STACK_SIZE,
        nullptr,
        1,  // Low priority
        &displayTaskHandle,
        1   // Core 1
    );
    Serial.println("     [OK] Display update task (Core 1, priority 1)");
    
    Serial.println();
    Serial.println("=====================================");
    Serial.println("System ready! Press START to begin...");
    Serial.println("\nSerial Commands:");
    Serial.println("  s - START measurement");
    Serial.println("  r - RESET analyzer");
    Serial.println("  c - CALIBRATE microphone");
    Serial.println("  i - PRINT statistics");
    Serial.println("  d - SET microphone distance");
    Serial.println("  a - SET microphone angle");
    Serial.println("  h - HELP");
    Serial.println("=====================================");
    Serial.println();
}

/**
 * Main loop - Handle serial commands and button input
 */
void loop() {
    if (Serial.available()) {
        char cmd = Serial.read();
        
        switch (cmd) {
            case 's':  // START measurement
                Serial.println("[CMD] START - Beginning 100-cycle measurement...");
                advancedAnalyzer.startMeasurement();
                advancedUI.setState(AdvancedMeasurementUI::STATE_MEASURING);
                break;
                
            case 'r':  // RESET
                Serial.println("[CMD] RESET - Clearing all data...");
                advancedAnalyzer.reset();
                advancedUI.setState(AdvancedMeasurementUI::STATE_IDLE);
                break;
                
            case 'c':  // CALIBRATE
                Serial.println("[CMD] CALIBRATE - Recalibrating microphone...");
                audio.calibrateReference();
                Serial.printf("     Gain: %.2f dB\n", audio.getGain());
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
                
            case 'i':  // Print statistics
                {
                    const auto& stats = advancedAnalyzer.getStats();
                    Serial.println("\n====== MEASUREMENT STATISTICS ======");
                    Serial.printf("CPS: %.2f Hz\n", stats.cps_calculated);
                    Serial.printf("Quality Score: %d%%\n", stats.quality_score);
                    Serial.printf("Consistency: %.1f%%\n", stats.consistency_percent);
                    Serial.printf("Waveforms Collected: %d\n", stats.samples_collected);
                    Serial.printf("Mean Period: %.2f ms\n", stats.mean_period_ms);
                    Serial.printf("Period Jitter: %.3f ms\n", stats.period_jitter_ms);
                    Serial.println();
                    Serial.println("====== HARMONIC ANALYSIS ======");
                    Serial.printf("Mean Harmonic 2/1 Ratio: %.4f\n", stats.mean_harmonic_2_1);
                    Serial.printf("Mean Harmonic 3/1 Ratio: %.4f\n", stats.mean_harmonic_3_1);
                    Serial.printf("Harmonic Consistency: %.1f%%\n", stats.harmonic_consistency);
                    Serial.println();
                    Serial.println("====== DECAY ANALYSIS ======");
                    Serial.printf("Mean Decay Rate: %.4f\n", stats.mean_decay_rate);
                    Serial.printf("Mean Q-Factor: %.2f\n", stats.mean_quality_factor);
                    Serial.println();
                    
                    if (stats.samples_collected > 0) {
                        const auto& wf = advancedAnalyzer.getLastWaveform();
                        Serial.println("====== LAST WAVEFORM ======");
                        Serial.printf("Measurement Points: %d\n", wf.point_count);
                        Serial.printf("Total Energy: %.2f\n", wf.total_energy);
                        Serial.printf("Peak Amplitude: %.2f dB\n", wf.peak_amplitude_db);
                        Serial.printf("Decay Rate: %.4f\n", wf.decay_rate);
                        Serial.printf("Q-Factor: %.2f\n", wf.quality_factor_q);
                        Serial.printf("THD: %.1f%%\n", wf.harmonic_content_percent);
                        Serial.println();
                        
                        Serial.println("====== MEASUREMENT POINTS ======");
                        for (size_t i = 0; i < wf.points.size(); i++) {
                            const auto& pt = wf.points[i];
                            const char* type_str = "";
                            switch (pt.type) {
                                case AdvancedImpactAnalyzer::BASELINE: type_str = "BASE"; break;
                                case AdvancedImpactAnalyzer::RISING_EDGE: type_str = "EDGE"; break;
                                case AdvancedImpactAnalyzer::PEAK: type_str = "PEAK"; break;
                                case AdvancedImpactAnalyzer::VALLEY: type_str = "VALY"; break;
                                case AdvancedImpactAnalyzer::END_IMPULSE: type_str = "END"; break;
                                default: type_str = "????"; break;
                            }
                            Serial.printf("%2d. %s T=%6.2fms A=%.4f dB=%6.2f Angle=%.1f\u00b0\n",
                                         i + 1, type_str, pt.time_ms,
                                         pt.amplitude_linear, pt.amplitude_db,
                                         pt.slope_angle_deg);
                        }
                    }
                    Serial.println("\n");
                }
                break;
                
            case 'd':  // Set microphone distance
                {
                    Serial.print("Enter microphone distance (10-500 mm): ");
                    // Simple serial input (in production, would need full command parsing)
                    // For now, just show current
                    float dist, angle;
                    advancedAnalyzer.getMicrophonePosition(dist, angle);
                    Serial.printf("Current: %.0f mm\n", dist);
                }
                break;
                
            case 'a':  // Set microphone angle
                {
                    Serial.print("Enter microphone angle (-180 to 180 degrees): ");
                    float dist, angle;
                    advancedAnalyzer.getMicrophonePosition(dist, angle);
                    Serial.printf("Current: %.0f degrees\n", angle);
                }
                break;
                
            case 'h':  // Help
                Serial.println("\n====== COMMAND REFERENCE ======");
                Serial.println("s      - START new measurement (100 cycles)");
                Serial.println("r      - RESET analyzer and clear data");
                Serial.println("c      - CALIBRATE microphone gain");
                Serial.println("+      - Increase gain by 5dB");
                Serial.println("-      - Decrease gain by 5dB");
                Serial.println("i      - Print current statistics");
                Serial.println("d      - Set microphone distance");
                Serial.println("a      - Set microphone angle");
                Serial.println("h      - Show this help");
                Serial.println();
                Serial.println("FEATURES:");
                Serial.println("- Multi-point waveform analysis");
                Serial.println("- Harmonic ratio analysis (H2/H1, H3/H1)");
                Serial.println("- Decay rate calculation");
                Serial.println("- Q-factor measurement");
                Serial.println("- Jitter analysis per measurement point");
                Serial.println("- Real-time UI with multiple views");
                Serial.println("- 0-250 CPS measurement range");
                Serial.println();
                break;
                
            default:
                break;
        }
    }
    
    delay(100);
}

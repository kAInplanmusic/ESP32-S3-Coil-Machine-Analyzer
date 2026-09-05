#include "coil_analyzer_core/AdvancedImpactAnalyzer.h"
#include "coil_analyzer_core/SignalProcessor.h"

#include <cmath>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <vector>

using namespace coil_analyzer_core;

namespace {

void require(bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

std::vector<std::int16_t> makeSine(std::size_t count, float freq, std::size_t sampleRate, float amplitude = 0.8f) {
    constexpr float pi = 3.14159265358979323846f;
    std::vector<std::int16_t> samples(count);
    for (std::size_t i = 0; i < count; ++i) {
        const float value = amplitude * std::sin(2.0f * pi * freq * static_cast<float>(i) / static_cast<float>(sampleRate));
        samples[i] = static_cast<std::int16_t>(value * 32767.0f);
    }
    return samples;
}

void testFftComputation() {
    SignalProcessor processor;
    auto samples = makeSine(4410, 430.0f, 44100);
    processor.processSamples(samples, 1'000'000);
    require(processor.computeFft(), "FFT computation failed");
    require(std::abs(processor.spectrumData().peakFrequencyHz - 430.0f) < 30.0f, "Peak frequency not detected");
}

void testPeakDetection() {
    SignalProcessor processor;
    std::vector<std::int16_t> samples(4410, 0);
    samples[100] = 20000;
    samples[101] = 32000;
    samples[102] = 12000;
    processor.processSamples(samples, 2'000'000);
    processor.computeFft();
    require(processor.detectPeaks(10, -20.0f, 2'000'000), "Peak detection failed");
    require(!processor.impactEvents().empty(), "No impact event captured");
}

void testHarmonicRatioCalculation() {
    AdvancedImpactAnalyzer analyzer;
    AdvancedImpactAnalyzer::ImpactWaveform waveform;
    waveform.points = {
        {1, AdvancedImpactAnalyzer::PointType::Peak, 0.0f, 1.0f},
        {2, AdvancedImpactAnalyzer::PointType::Valley, 0.5f, 0.2f},
        {3, AdvancedImpactAnalyzer::PointType::Peak, 1.0f, 0.5f},
        {4, AdvancedImpactAnalyzer::PointType::Valley, 1.5f, 0.2f},
        {5, AdvancedImpactAnalyzer::PointType::Peak, 2.0f, 0.25f},
    };
    analyzer.calculateHarmonics(waveform);
    require(std::abs(waveform.harmonicRatio2To1 - 0.5f) < 0.01f, "H2/H1 ratio incorrect");
    require(std::abs(waveform.harmonicRatio3To1 - 0.25f) < 0.01f, "H3/H1 ratio incorrect");
}

void testQualityScoreComputation() {
    AdvancedImpactAnalyzer analyzer;
    AdvancedImpactAnalyzer::ImpactWaveform waveform;
    waveform.timestampMs = 1000;
    waveform.points = {
        {1, AdvancedImpactAnalyzer::PointType::Peak, 0.0f, 1.0f},
        {2, AdvancedImpactAnalyzer::PointType::Peak, 1.0f, 0.5f},
        {3, AdvancedImpactAnalyzer::PointType::Peak, 2.0f, 0.25f},
    };
    analyzer.calculateHarmonics(waveform);
    analyzer.calculateQualityScore(waveform);
    require(waveform.overallQuality >= 70, "Quality score unexpectedly low");
}

}  // namespace

int main() {
    try {
        testFftComputation();
        testPeakDetection();
        testHarmonicRatioCalculation();
        testQualityScoreComputation();
        std::cout << "All coil_analyzer_core tests passed\n";
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << ex.what() << '\n';
        return 1;
    }
}

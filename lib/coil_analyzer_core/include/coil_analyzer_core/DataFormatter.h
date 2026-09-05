#pragma once

#include <string>

namespace coil_analyzer_core {

class SignalProcessor;
class AdvancedImpactAnalyzer;
class CalibrationManager;

class DataFormatter {
public:
    virtual ~DataFormatter() = default;
    virtual std::string formatSignalProcessor(const SignalProcessor& processor) const = 0;
    virtual std::string formatImpactAnalyzer(const AdvancedImpactAnalyzer& analyzer) const = 0;
    virtual std::string formatCalibration(const CalibrationManager& calibration) const = 0;
};

}  // namespace coil_analyzer_core

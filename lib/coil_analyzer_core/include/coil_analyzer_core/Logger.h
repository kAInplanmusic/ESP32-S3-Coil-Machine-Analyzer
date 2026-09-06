#pragma once

#include <string_view>

namespace coil_analyzer_core {

class Logger {
public:
    virtual ~Logger() = default;
    virtual void info(std::string_view message) = 0;
    virtual void warn(std::string_view message) = 0;
    virtual void error(std::string_view message) = 0;
};

class NullLogger final : public Logger {
public:
    void info(std::string_view) override {}
    void warn(std::string_view) override {}
    void error(std::string_view) override {}
};

}  // namespace coil_analyzer_core

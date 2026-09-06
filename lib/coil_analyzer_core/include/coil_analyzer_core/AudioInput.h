#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace coil_analyzer_core {

class AudioInput {
public:
    virtual ~AudioInput() = default;
    virtual std::size_t readSamples(std::int16_t* destination, std::size_t maxSamples) = 0;
    virtual float getGainDb() const = 0;
    virtual void setGainDb(float gainDb) = 0;

    std::vector<std::int16_t> readSamples(std::size_t maxSamples) {
        std::vector<std::int16_t> buffer(maxSamples);
        const auto count = readSamples(buffer.data(), maxSamples);
        buffer.resize(count);
        return buffer;
    }
};

}  // namespace coil_analyzer_core

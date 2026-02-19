#pragma once

#include <array>

namespace audiotomidi {

class BeatDetector
{
public:
    struct Params
    {
        float sensitivity = 60.0f;
        float minGapMs = 120.0f;
        bool focusLow = true;
    };

    struct TriggerEvent
    {
        int sampleOffset = 0;
        float strength = 0.0f;
    };

    struct TriggerBuffer
    {
        std::array<TriggerEvent, 64> events{};
        int count = 0;
        float envelope = 0.0f;
        float threshold = 0.0f;
    };

    void prepare(double sampleRate) noexcept;
    void reset() noexcept;
    void processBlock(const float* monoSamples, int numSamples, const Params& params, TriggerBuffer& out) noexcept;

private:
    double sampleRateHz = 44100.0;
    float envelope = 0.0f;
    float noiseFloor = 0.0f;
    float lowPassed = 0.0f;
    bool wasAboveThreshold = false;
    int samplesSinceLastTrigger = 0;
};

} // namespace audiotomidi

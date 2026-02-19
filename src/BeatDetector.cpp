#include "BeatDetector.h"

#include <algorithm>
#include <cmath>

namespace audiotomidi {

namespace
{
constexpr float kPi = 3.14159265358979323846f;
constexpr float kMinThreshold = 0.0035f;
}

void BeatDetector::prepare(double sr) noexcept
{
    sampleRateHz = sr > 0.0 ? sr : 44100.0;
    reset();
}

void BeatDetector::reset() noexcept
{
    envelope = 0.0f;
    noiseFloor = 0.0f;
    lowPassed = 0.0f;
    wasAboveThreshold = false;
    samplesSinceLastTrigger = static_cast<int>(sampleRateHz);
}

void BeatDetector::processBlock(const float* monoSamples, int numSamples, const Params& params, TriggerBuffer& out) noexcept
{
    out.count = 0;

    const auto gapSamples = std::max(1, static_cast<int>(params.minGapMs * 0.001f * static_cast<float>(sampleRateHz)));
    const float sensitivity = std::clamp(params.sensitivity, 0.0f, 100.0f) * 0.01f;

    const float envTimeMs = 8.0f;
    const float envAlpha = 1.0f - std::exp(-1.0f / (0.001f * envTimeMs * static_cast<float>(sampleRateHz)));

    const float noiseTimeMs = 350.0f;
    const float noiseAlpha = 1.0f - std::exp(-1.0f / (0.001f * noiseTimeMs * static_cast<float>(sampleRateHz)));

    const float lowHz = 180.0f;
    const float lowAlpha = 1.0f - std::exp(-2.0f * kPi * lowHz / static_cast<float>(sampleRateHz));

    for (int i = 0; i < numSamples; ++i)
    {
        float x = monoSamples[i];

        if (params.focusLow)
        {
            lowPassed += lowAlpha * (x - lowPassed);
            x = lowPassed;
        }

        const float rectified = std::abs(x);
        envelope += envAlpha * (rectified - envelope);

        const float noiseTarget = std::min(envelope, noiseFloor + 0.08f);
        noiseFloor += noiseAlpha * (noiseTarget - noiseFloor);

        const float thresholdLift = (1.0f - sensitivity) * 0.18f;
        const float threshold = std::max(kMinThreshold, noiseFloor + thresholdLift);
        const bool above = envelope >= threshold;

        ++samplesSinceLastTrigger;

        if (!wasAboveThreshold && above && samplesSinceLastTrigger >= gapSamples)
        {
            if (out.count < static_cast<int>(out.events.size()))
            {
                auto& event = out.events[static_cast<size_t>(out.count++)];
                event.sampleOffset = i;
                event.strength = std::clamp((envelope - threshold) * 8.0f, 0.0f, 1.0f);
            }
            samplesSinceLastTrigger = 0;
        }

        wasAboveThreshold = above;
        out.envelope = envelope;
        out.threshold = threshold;
    }
}

} // namespace audiotomidi

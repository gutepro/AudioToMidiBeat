#pragma once

#include <array>

#include <juce_audio_basics/juce_audio_basics.h>

#include "BeatDetector.h"

namespace audiotomidi {

enum class VelocityMode
{
    Fixed = 0,
    Dynamic = 1
};

struct MidiEngineParams
{
    int noteNumber = 36;
    int midiChannel = 1;
    int noteLengthMs = 30;
    VelocityMode velocityMode = VelocityMode::Fixed;
    int fixedVelocity = 100;
};

class MidiEngine
{
public:
    void prepare(double sampleRate) noexcept;
    void reset() noexcept;

    void process(const BeatDetector::TriggerBuffer& triggers,
                 juce::MidiBuffer& midi,
                 int numSamples,
                 const MidiEngineParams& params) noexcept;

private:
    struct PendingNoteOff
    {
        bool active = false;
        int samplesRemaining = 0;
        int noteNumber = 0;
        int midiChannel = 1;
    };

    void processPending(juce::MidiBuffer& midi, int numSamples) noexcept;
    void addPending(int samplesRemaining, int noteNumber, int midiChannel) noexcept;

    std::array<PendingNoteOff, 128> pending{};
    double sampleRateHz = 44100.0;
};

} // namespace audiotomidi

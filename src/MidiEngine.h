#pragma once

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
    void prepare(double) {}
    void reset() {}
};

} // namespace audiotomidi

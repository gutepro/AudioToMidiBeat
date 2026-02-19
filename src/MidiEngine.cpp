#include "MidiEngine.h"

#include <algorithm>

namespace audiotomidi {

void MidiEngine::prepare(double sampleRate) noexcept
{
    sampleRateHz = sampleRate > 0.0 ? sampleRate : 44100.0;
    reset();
}

void MidiEngine::reset() noexcept
{
    for (auto& note : pending)
        note = {};
}

void MidiEngine::processPending(juce::MidiBuffer& midi, int numSamples) noexcept
{
    for (auto& note : pending)
    {
        if (!note.active)
            continue;

        if (note.samplesRemaining < numSamples)
        {
            midi.addEvent(juce::MidiMessage::noteOff(note.midiChannel, note.noteNumber), note.samplesRemaining);
            note = {};
        }
        else
        {
            note.samplesRemaining -= numSamples;
        }
    }
}

void MidiEngine::addPending(int samplesRemaining, int noteNumber, int midiChannel) noexcept
{
    for (auto& note : pending)
    {
        if (!note.active)
        {
            note.active = true;
            note.samplesRemaining = samplesRemaining;
            note.noteNumber = noteNumber;
            note.midiChannel = midiChannel;
            return;
        }
    }

    pending[0].active = true;
    pending[0].samplesRemaining = samplesRemaining;
    pending[0].noteNumber = noteNumber;
    pending[0].midiChannel = midiChannel;
}

void MidiEngine::process(const BeatDetector::TriggerBuffer& triggers,
                         juce::MidiBuffer& midi,
                         int numSamples,
                         const MidiEngineParams& params) noexcept
{
    processPending(midi, numSamples);

    const auto noteNumber = std::clamp(params.noteNumber, 0, 127);
    const auto channel = std::clamp(params.midiChannel, 1, 16);
    const auto fixedVelocity = std::clamp(params.fixedVelocity, 0, 127);
    const int noteLengthSamples = std::max(1, static_cast<int>(0.001 * static_cast<double>(params.noteLengthMs) * sampleRateHz));

    for (int i = 0; i < triggers.count; ++i)
    {
        const auto& event = triggers.events[static_cast<size_t>(i)];
        const int offset = std::clamp(event.sampleOffset, 0, std::max(0, numSamples - 1));

        int velocity = fixedVelocity;
        if (params.velocityMode == VelocityMode::Dynamic)
            velocity = std::clamp(static_cast<int>(juce::jmap(event.strength, 0.0f, 1.0f, 25.0f, 127.0f)), 1, 127);

        midi.addEvent(juce::MidiMessage::noteOn(channel, noteNumber, static_cast<juce::uint8>(velocity)), offset);

        const int noteOffOffset = offset + noteLengthSamples;
        if (noteOffOffset < numSamples)
        {
            midi.addEvent(juce::MidiMessage::noteOff(channel, noteNumber), noteOffOffset);
        }
        else
        {
            addPending(noteOffOffset - numSamples, noteNumber, channel);
        }
    }
}

} // namespace audiotomidi

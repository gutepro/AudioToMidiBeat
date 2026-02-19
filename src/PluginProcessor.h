#pragma once

#include <atomic>

#include <juce_audio_processors/juce_audio_processors.h>

#include "BeatDetector.h"
#include "MidiEngine.h"

namespace paramids {
static constexpr auto sensitivity = "sensitivity";
static constexpr auto minGapMs = "minGapMs";
static constexpr auto noteNumber = "noteNumber";
static constexpr auto midiChannel = "midiChannel";
static constexpr auto noteLengthMs = "noteLengthMs";
static constexpr auto velocityMode = "velocityMode";
static constexpr auto fixedVelocity = "fixedVelocity";
static constexpr auto focusLow = "focusLow";
}

class AudioToMidiBeatAudioProcessor : public juce::AudioProcessor
{
public:
    AudioToMidiBeatAudioProcessor();
    ~AudioToMidiBeatAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getValueTreeState() noexcept { return apvts; }
    float getInputLevel() const noexcept { return inputLevelAtomic.load(std::memory_order_relaxed); }
    bool consumeTriggerFlash() noexcept;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

private:
    juce::AudioProcessorValueTreeState apvts;
    audiotomidi::BeatDetector detector;
    audiotomidi::MidiEngine midiEngine;

    juce::HeapBlock<float> monoBuffer;
    int monoBufferSize = 0;

    std::atomic<float> inputLevelAtomic { 0.0f };
    std::atomic<bool> triggerFlashAtomic { false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioToMidiBeatAudioProcessor)
};

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter();

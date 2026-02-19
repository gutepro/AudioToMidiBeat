#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include "PluginProcessor.h"

class AudioToMidiBeatAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit AudioToMidiBeatAudioProcessorEditor(AudioToMidiBeatAudioProcessor&);
    ~AudioToMidiBeatAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    AudioToMidiBeatAudioProcessor& audioProcessor;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioToMidiBeatAudioProcessorEditor)
};

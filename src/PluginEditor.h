#pragma once

#include <array>
#include <memory>

#include <juce_gui_extra/juce_gui_extra.h>

#include "PluginProcessor.h"

class AudioToMidiBeatAudioProcessorEditor : public juce::AudioProcessorEditor,
                                            private juce::Timer
{
public:
    explicit AudioToMidiBeatAudioProcessorEditor(AudioToMidiBeatAudioProcessor&);
    ~AudioToMidiBeatAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    AudioToMidiBeatAudioProcessor& audioProcessor;

    juce::Label titleLabel;
    juce::Label modeLabel;

    juce::Slider sensitivitySlider;
    juce::Slider minGapSlider;
    juce::Slider noteSlider;
    juce::Slider channelSlider;
    juce::Slider noteLengthSlider;
    juce::Slider fixedVelocitySlider;

    juce::ComboBox velocityModeBox;
    juce::ToggleButton focusLowToggle;

    juce::TextButton startStopButton { "Stop" };

    juce::Label levelLabel;
    juce::Label triggerLabel;

    bool running = true;
    int triggerFrames = 0;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    std::unique_ptr<SliderAttachment> sensitivityAttachment;
    std::unique_ptr<SliderAttachment> minGapAttachment;
    std::unique_ptr<SliderAttachment> noteAttachment;
    std::unique_ptr<SliderAttachment> channelAttachment;
    std::unique_ptr<SliderAttachment> noteLengthAttachment;
    std::unique_ptr<SliderAttachment> fixedVelocityAttachment;
    std::unique_ptr<ComboAttachment> velocityModeAttachment;
    std::unique_ptr<ButtonAttachment> focusLowAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioToMidiBeatAudioProcessorEditor)
};

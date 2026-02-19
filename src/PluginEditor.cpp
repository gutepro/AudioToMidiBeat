#include "PluginEditor.h"

AudioToMidiBeatAudioProcessorEditor::AudioToMidiBeatAudioProcessorEditor(AudioToMidiBeatAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setSize(520, 360);
}

AudioToMidiBeatAudioProcessorEditor::~AudioToMidiBeatAudioProcessorEditor() = default;

void AudioToMidiBeatAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
    g.setColour(juce::Colours::white);
    g.setFont(18.0f);
    g.drawFittedText("AudioToMidiBeat", getLocalBounds(), juce::Justification::centredTop, 1);
}

void AudioToMidiBeatAudioProcessorEditor::resized() {}

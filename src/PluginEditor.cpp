#include "PluginEditor.h"

namespace
{
void configureSlider(juce::Slider& slider, const juce::String& suffix)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 72, 20);
    slider.setTextValueSuffix(suffix);
}
} // namespace

AudioToMidiBeatAudioProcessorEditor::AudioToMidiBeatAudioProcessorEditor(AudioToMidiBeatAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setSize(720, 440);

    titleLabel.setText("AudioToMidiBeat", juce::dontSendNotification);
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    titleLabel.setFont(juce::FontOptions(28.0f, juce::Font::bold));
    addAndMakeVisible(titleLabel);

    modeLabel.setJustificationType(juce::Justification::centredRight);
    modeLabel.setText(audioProcessor.wrapperType == juce::AudioProcessor::wrapperType_Standalone ? "Standalone wrapper" : "VST3 plugin", juce::dontSendNotification);
    addAndMakeVisible(modeLabel);

    configureSlider(sensitivitySlider, "");
    configureSlider(minGapSlider, " ms");
    configureSlider(noteSlider, "");
    configureSlider(channelSlider, "");
    configureSlider(noteLengthSlider, " ms");
    configureSlider(fixedVelocitySlider, "");

    sensitivitySlider.setName("Sensitivity");
    minGapSlider.setName("Min Gap");
    noteSlider.setName("Note");
    channelSlider.setName("Channel");
    noteLengthSlider.setName("Note Len");
    fixedVelocitySlider.setName("Velocity");

    addAndMakeVisible(sensitivitySlider);
    addAndMakeVisible(minGapSlider);
    addAndMakeVisible(noteSlider);
    addAndMakeVisible(channelSlider);
    addAndMakeVisible(noteLengthSlider);
    addAndMakeVisible(fixedVelocitySlider);

    velocityModeBox.addItem("Fixed", 1);
    velocityModeBox.addItem("Dynamic", 2);
    addAndMakeVisible(velocityModeBox);

    focusLowToggle.setButtonText("Focus Low (180Hz)");
    addAndMakeVisible(focusLowToggle);

    startStopButton.onClick = [this]
    {
        running = !running;
        startStopButton.setButtonText(running ? "Stop" : "Start");
    };
    addAndMakeVisible(startStopButton);

    levelLabel.setText("Input: 0%", juce::dontSendNotification);
    addAndMakeVisible(levelLabel);

    triggerLabel.setText("Trigger", juce::dontSendNotification);
    triggerLabel.setColour(juce::Label::backgroundColourId, juce::Colours::darkred.withAlpha(0.8f));
    triggerLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    triggerLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(triggerLabel);

    auto& apvts = audioProcessor.getValueTreeState();
    sensitivityAttachment = std::make_unique<SliderAttachment>(apvts, paramids::sensitivity, sensitivitySlider);
    minGapAttachment = std::make_unique<SliderAttachment>(apvts, paramids::minGapMs, minGapSlider);
    noteAttachment = std::make_unique<SliderAttachment>(apvts, paramids::noteNumber, noteSlider);
    channelAttachment = std::make_unique<SliderAttachment>(apvts, paramids::midiChannel, channelSlider);
    noteLengthAttachment = std::make_unique<SliderAttachment>(apvts, paramids::noteLengthMs, noteLengthSlider);
    fixedVelocityAttachment = std::make_unique<SliderAttachment>(apvts, paramids::fixedVelocity, fixedVelocitySlider);
    velocityModeAttachment = std::make_unique<ComboAttachment>(apvts, paramids::velocityMode, velocityModeBox);
    focusLowAttachment = std::make_unique<ButtonAttachment>(apvts, paramids::focusLow, focusLowToggle);

    startTimerHz(30);
}

AudioToMidiBeatAudioProcessorEditor::~AudioToMidiBeatAudioProcessorEditor() = default;

void AudioToMidiBeatAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour::fromRGB(18, 20, 26));
    g.setColour(juce::Colour::fromRGB(35, 40, 52));
    g.fillRoundedRectangle(getLocalBounds().reduced(12).toFloat(), 14.0f);

    g.setColour(juce::Colours::white.withAlpha(0.72f));
    for (auto* slider : std::array<juce::Slider*, 6> { &sensitivitySlider, &minGapSlider, &noteSlider, &channelSlider, &noteLengthSlider, &fixedVelocitySlider })
    {
        const auto bounds = slider->getBounds().translated(0, -18);
        g.drawFittedText(slider->getName(), bounds, juce::Justification::centred, 1);
    }
}

void AudioToMidiBeatAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(24);

    auto header = area.removeFromTop(48);
    titleLabel.setBounds(header.removeFromLeft(320));
    modeLabel.setBounds(header);

    auto topControls = area.removeFromTop(170);
    auto cellW = topControls.getWidth() / 3;

    sensitivitySlider.setBounds(topControls.removeFromLeft(cellW).reduced(6));
    minGapSlider.setBounds(topControls.removeFromLeft(cellW).reduced(6));
    noteSlider.setBounds(topControls.reduced(6));

    auto midControls = area.removeFromTop(170);
    cellW = midControls.getWidth() / 3;

    channelSlider.setBounds(midControls.removeFromLeft(cellW).reduced(6));
    noteLengthSlider.setBounds(midControls.removeFromLeft(cellW).reduced(6));
    fixedVelocitySlider.setBounds(midControls.reduced(6));

    auto footer = area.removeFromTop(56);
    velocityModeBox.setBounds(footer.removeFromLeft(160).reduced(2));
    focusLowToggle.setBounds(footer.removeFromLeft(200).reduced(2));
    startStopButton.setBounds(footer.removeFromLeft(110).reduced(2));
    levelLabel.setBounds(footer.removeFromLeft(120).reduced(2));
    triggerLabel.setBounds(footer.removeFromLeft(90).reduced(2));
}

void AudioToMidiBeatAudioProcessorEditor::timerCallback()
{
    const auto level = audioProcessor.getInputLevel();
    levelLabel.setText("Input: " + juce::String(static_cast<int>(juce::jlimit(0.0f, 1.0f, level) * 100.0f)) + "%", juce::dontSendNotification);

    if (audioProcessor.consumeTriggerFlash())
        triggerFrames = 4;

    if (triggerFrames > 0)
    {
        --triggerFrames;
        triggerLabel.setColour(juce::Label::backgroundColourId, juce::Colours::limegreen.withAlpha(0.9f));
    }
    else
    {
        triggerLabel.setColour(juce::Label::backgroundColourId, juce::Colours::darkred.withAlpha(0.8f));
    }

    if (!running)
        triggerLabel.setColour(juce::Label::backgroundColourId, juce::Colours::dimgrey);

    triggerLabel.repaint();
}

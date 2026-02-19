#include "PluginProcessor.h"
#include "PluginEditor.h"

AudioToMidiBeatAudioProcessor::AudioToMidiBeatAudioProcessor()
    : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
                                     .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
}

AudioToMidiBeatAudioProcessor::~AudioToMidiBeatAudioProcessor() = default;

void AudioToMidiBeatAudioProcessor::prepareToPlay(double, int) {}
void AudioToMidiBeatAudioProcessor::releaseResources() {}

bool AudioToMidiBeatAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    return layouts.getMainInputChannelSet() == layouts.getMainOutputChannelSet();
}

void AudioToMidiBeatAudioProcessor::processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) {}

juce::AudioProcessorEditor* AudioToMidiBeatAudioProcessor::createEditor() { return new AudioToMidiBeatAudioProcessorEditor(*this); }
bool AudioToMidiBeatAudioProcessor::hasEditor() const { return true; }

const juce::String AudioToMidiBeatAudioProcessor::getName() const { return JucePlugin_Name; }
bool AudioToMidiBeatAudioProcessor::acceptsMidi() const { return false; }
bool AudioToMidiBeatAudioProcessor::producesMidi() const { return true; }
bool AudioToMidiBeatAudioProcessor::isMidiEffect() const { return false; }
double AudioToMidiBeatAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int AudioToMidiBeatAudioProcessor::getNumPrograms() { return 1; }
int AudioToMidiBeatAudioProcessor::getCurrentProgram() { return 0; }
void AudioToMidiBeatAudioProcessor::setCurrentProgram(int) {}
const juce::String AudioToMidiBeatAudioProcessor::getProgramName(int) { return {}; }
void AudioToMidiBeatAudioProcessor::changeProgramName(int, const juce::String&) {}

void AudioToMidiBeatAudioProcessor::getStateInformation(juce::MemoryBlock&) {}
void AudioToMidiBeatAudioProcessor::setStateInformation(const void*, int) {}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AudioToMidiBeatAudioProcessor();
}

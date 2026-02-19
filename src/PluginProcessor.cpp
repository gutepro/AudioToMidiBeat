#include "PluginProcessor.h"

#include "PluginEditor.h"

AudioToMidiBeatAudioProcessor::AudioToMidiBeatAudioProcessor()
    : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
                                     .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "PARAMETERS", createParameterLayout())
{
}

AudioToMidiBeatAudioProcessor::~AudioToMidiBeatAudioProcessor() = default;

juce::AudioProcessorValueTreeState::ParameterLayout AudioToMidiBeatAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>(paramids::sensitivity, "Sensitivity", 0.0f, 100.0f, 60.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(paramids::minGapMs, "Min Gap (ms)", 50.0f, 300.0f, 120.0f));
    params.push_back(std::make_unique<juce::AudioParameterInt>(paramids::noteNumber, "Note Number", 0, 127, 36));
    params.push_back(std::make_unique<juce::AudioParameterInt>(paramids::midiChannel, "MIDI Channel", 1, 16, 1));
    params.push_back(std::make_unique<juce::AudioParameterInt>(paramids::noteLengthMs, "Note Length (ms)", 10, 120, 30));

    juce::StringArray velocityChoices { "Fixed", "Dynamic" };
    params.push_back(std::make_unique<juce::AudioParameterChoice>(paramids::velocityMode, "Velocity Mode", velocityChoices, 0));

    params.push_back(std::make_unique<juce::AudioParameterInt>(paramids::fixedVelocity, "Fixed Velocity", 0, 127, 100));
    params.push_back(std::make_unique<juce::AudioParameterBool>(paramids::focusLow, "Focus Low", true));

    return { params.begin(), params.end() };
}

void AudioToMidiBeatAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    detector.prepare(sampleRate);
    midiEngine.prepare(sampleRate);

    monoBuffer.allocate(static_cast<size_t>(samplesPerBlock), true);
    monoBufferSize = samplesPerBlock;
}

void AudioToMidiBeatAudioProcessor::releaseResources()
{
    midiEngine.reset();
    detector.reset();
}

bool AudioToMidiBeatAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainInputChannelSet().isDisabled())
        return false;

    return layouts.getMainInputChannelSet() == layouts.getMainOutputChannelSet();
}

void AudioToMidiBeatAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    const auto totalNumInputChannels = getTotalNumInputChannels();
    const auto totalNumOutputChannels = getTotalNumOutputChannels();
    const auto numSamples = buffer.getNumSamples();

    for (int i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, numSamples);

    if (numSamples > monoBufferSize)
    {
        monoBuffer.allocate(static_cast<size_t>(numSamples), false);
        monoBufferSize = numSamples;
    }

    float peak = 0.0f;
    for (int s = 0; s < numSamples; ++s)
    {
        float mono = 0.0f;
        for (int c = 0; c < totalNumInputChannels; ++c)
            mono += buffer.getReadPointer(c)[s];

        mono *= (totalNumInputChannels > 0 ? 1.0f / static_cast<float>(totalNumInputChannels) : 1.0f);
        monoBuffer[s] = mono;
        peak = std::max(peak, std::abs(mono));
    }

    inputLevelAtomic.store(peak, std::memory_order_relaxed);

    audiotomidi::BeatDetector::Params detParams;
    detParams.sensitivity = apvts.getRawParameterValue(paramids::sensitivity)->load();
    detParams.minGapMs = apvts.getRawParameterValue(paramids::minGapMs)->load();
    detParams.focusLow = apvts.getRawParameterValue(paramids::focusLow)->load() >= 0.5f;

    audiotomidi::BeatDetector::TriggerBuffer triggers;
    detector.processBlock(monoBuffer.get(), numSamples, detParams, triggers);

    if (triggers.count > 0)
        triggerFlashAtomic.store(true, std::memory_order_relaxed);

    audiotomidi::MidiEngineParams midiParams;
    midiParams.noteNumber = static_cast<int>(apvts.getRawParameterValue(paramids::noteNumber)->load());
    midiParams.midiChannel = static_cast<int>(apvts.getRawParameterValue(paramids::midiChannel)->load());
    midiParams.noteLengthMs = static_cast<int>(apvts.getRawParameterValue(paramids::noteLengthMs)->load());
    midiParams.velocityMode = static_cast<int>(apvts.getRawParameterValue(paramids::velocityMode)->load()) == 0
                                ? audiotomidi::VelocityMode::Fixed
                                : audiotomidi::VelocityMode::Dynamic;
    midiParams.fixedVelocity = static_cast<int>(apvts.getRawParameterValue(paramids::fixedVelocity)->load());

    midiMessages.clear();
    midiEngine.process(triggers, midiMessages, numSamples, midiParams);
}

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

void AudioToMidiBeatAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    const auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void AudioToMidiBeatAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr && xmlState->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

bool AudioToMidiBeatAudioProcessor::consumeTriggerFlash() noexcept
{
    return triggerFlashAtomic.exchange(false, std::memory_order_relaxed);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AudioToMidiBeatAudioProcessor();
}

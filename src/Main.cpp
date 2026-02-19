#include <array>
#include <memory>

#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_gui_extra/juce_gui_extra.h>

#include "BeatDetector.h"
#include "MidiEngine.h"

namespace
{
class LevelMeter : public juce::Component
{
public:
    void setLevel(float newLevel)
    {
        level = juce::jlimit(0.0f, 1.0f, newLevel);
        repaint();
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colours::black.withAlpha(0.25f));
        auto bounds = getLocalBounds().reduced(2);
        g.setColour(juce::Colours::limegreen.withAlpha(0.85f));
        g.fillRect(bounds.removeFromLeft(static_cast<int>(bounds.getWidth() * level)));
        g.setColour(juce::Colours::white.withAlpha(0.5f));
        g.drawRect(getLocalBounds(), 1);
    }

private:
    float level = 0.0f;
};

class TriggerLed : public juce::Component
{
public:
    void setTriggered(bool shouldFlash)
    {
        if (shouldFlash)
            frames = 5;

        if (frames > 0)
            --frames;

        repaint();
    }

    void paint(juce::Graphics& g) override
    {
        g.setColour(frames > 0 ? juce::Colours::limegreen : juce::Colours::darkred);
        g.fillEllipse(getLocalBounds().toFloat());
    }

private:
    int frames = 0;
};

class StandaloneMainComponent : public juce::Component,
                                private juce::AudioIODeviceCallback,
                                private juce::ComboBox::Listener,
                                private juce::Timer
{
public:
    StandaloneMainComponent()
        : audioSelector(deviceManager,
                        1,
                        2,
                        0,
                        0,
                        true,
                        false,
                        false,
                        false)
    {
        setSize(900, 640);

        juce::PropertiesFile::Options options;
        options.applicationName = "AudioToMidiBeat";
        options.filenameSuffix = "settings";
        options.folderName = "AudioToMidiBeat";
        options.osxLibrarySubFolder = "Application Support";
        appProperties.setStorageParameters(options);

        if (auto* storage = appProperties.getUserSettings())
            restoreSettings(*storage);

        deviceManager.initialiseWithDefaultDevices(2, 0);
        if (settings.audioState.isNotEmpty())
        {
            std::unique_ptr<juce::XmlElement> stateXml(juce::XmlDocument::parse(settings.audioState));
            if (stateXml != nullptr)
                deviceManager.initialise(2, 0, stateXml.get(), true);
        }
        deviceManager.addAudioCallback(this);

        addAndMakeVisible(audioSelector);

        midiOutLabel.setText("MIDI Output", juce::dontSendNotification);
        addAndMakeVisible(midiOutLabel);

        midiOutputBox.addListener(this);
        addAndMakeVisible(midiOutputBox);

        startStopButton.setButtonText("Stop");
        startStopButton.onClick = [this]
        {
            running = !running;
            startStopButton.setButtonText(running ? "Stop" : "Start");
        };
        addAndMakeVisible(startStopButton);

        refreshButton.setButtonText("Refresh Devices");
        refreshButton.onClick = [this] { refreshDevices(); };
        addAndMakeVisible(refreshButton);

        addAndMakeVisible(levelLabel);
        levelLabel.setText("Input Level", juce::dontSendNotification);

        addAndMakeVisible(levelMeter);
        addAndMakeVisible(triggerLabel);
        triggerLabel.setText("Trigger", juce::dontSendNotification);
        addAndMakeVisible(triggerLed);

        auto addSlider = [this](juce::Slider& slider, juce::Label& label, const juce::String& text, double min, double max, double step, double val)
        {
            slider.setRange(min, max, step);
            slider.setValue(val);
            slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
            slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 64, 20);
            label.setText(text, juce::dontSendNotification);
            label.setJustificationType(juce::Justification::centred);
            addAndMakeVisible(slider);
            addAndMakeVisible(label);
        };

        addSlider(sensitivitySlider, sensitivityLabel, "Sensitivity", 0, 100, 1, settings.sensitivity);
        addSlider(minGapSlider, minGapLabel, "MinGapMs", 50, 300, 1, settings.minGapMs);
        addSlider(noteSlider, noteLabel, "NoteNumber", 0, 127, 1, settings.noteNumber);
        addSlider(channelSlider, channelLabel, "MidiChannel", 1, 16, 1, settings.midiChannel);
        addSlider(noteLenSlider, noteLenLabel, "NoteLengthMs", 10, 120, 1, settings.noteLengthMs);
        addSlider(velocitySlider, velocityLabel, "FixedVelocity", 0, 127, 1, settings.fixedVelocity);

        velocityModeBox.addItem("Fixed", 1);
        velocityModeBox.addItem("Dynamic", 2);
        velocityModeBox.setSelectedId(settings.velocityModeId);
        addAndMakeVisible(velocityModeBox);

        focusLowToggle.setButtonText("FocusLow");
        focusLowToggle.setToggleState(settings.focusLow, juce::dontSendNotification);
        addAndMakeVisible(focusLowToggle);

        refreshDevices();
        if (settings.running == false)
        {
            running = false;
            startStopButton.setButtonText("Start");
        }

        startTimerHz(30);
    }

    ~StandaloneMainComponent() override
    {
        if (auto* storage = appProperties.getUserSettings())
            saveSettings(*storage);

        deviceManager.removeAudioCallback(this);
        midiOutput.reset();
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced(12);

        auto top = area.removeFromTop(36);
        midiOutLabel.setBounds(top.removeFromLeft(100));
        midiOutputBox.setBounds(top.removeFromLeft(270).reduced(2));
        refreshButton.setBounds(top.removeFromLeft(140).reduced(2));
        startStopButton.setBounds(top.removeFromLeft(100).reduced(2));

        auto meter = area.removeFromTop(36);
        levelLabel.setBounds(meter.removeFromLeft(100));
        levelMeter.setBounds(meter.removeFromLeft(280).reduced(4));
        triggerLabel.setBounds(meter.removeFromLeft(70));
        triggerLed.setBounds(meter.removeFromLeft(26).reduced(3));

        auto controls = area.removeFromTop(200);
        const int colW = controls.getWidth() / 6;
        std::array<juce::Slider*, 6> sliders { &sensitivitySlider, &minGapSlider, &noteSlider, &channelSlider, &noteLenSlider, &velocitySlider };
        std::array<juce::Label*, 6> labels { &sensitivityLabel, &minGapLabel, &noteLabel, &channelLabel, &noteLenLabel, &velocityLabel };

        for (int i = 0; i < 6; ++i)
        {
            auto col = controls.removeFromLeft(colW).reduced(2);
            labels[static_cast<size_t>(i)]->setBounds(col.removeFromTop(20));
            sliders[static_cast<size_t>(i)]->setBounds(col);
        }

        auto toggles = area.removeFromTop(40);
        velocityModeBox.setBounds(toggles.removeFromLeft(160).reduced(2));
        focusLowToggle.setBounds(toggles.removeFromLeft(130).reduced(2));

        audioSelector.setBounds(area.reduced(2));
    }

private:
    void audioDeviceAboutToStart(juce::AudioIODevice* device) override
    {
        const auto sr = device != nullptr ? device->getCurrentSampleRate() : 44100.0;
        const auto maxBlock = device != nullptr ? device->getCurrentBufferSizeSamples() : 512;

        detector.prepare(sr);
        midiEngine.prepare(sr);

        monoBuffer.allocate(static_cast<size_t>(maxBlock), true);
        monoBufferSize = maxBlock;
        currentSampleRate = sr;
    }

    void audioDeviceStopped() override
    {
        detector.reset();
        midiEngine.reset();
    }

    void audioDeviceIOCallbackWithContext(const float* const* inputChannelData,
                                          int numInputChannels,
                                          float* const* outputChannelData,
                                          int numOutputChannels,
                                          int numSamples,
                                          const juce::AudioIODeviceCallbackContext&) override
    {
        juce::ignoreUnused(outputChannelData, numOutputChannels);

        if (numSamples > monoBufferSize)
        {
            monoBuffer.allocate(static_cast<size_t>(numSamples), false);
            monoBufferSize = numSamples;
        }

        float peak = 0.0f;
        for (int s = 0; s < numSamples; ++s)
        {
            float mono = 0.0f;
            for (int ch = 0; ch < numInputChannels; ++ch)
            {
                const auto* in = inputChannelData[ch];
                mono += in != nullptr ? in[s] : 0.0f;
            }

            mono *= (numInputChannels > 0 ? 1.0f / static_cast<float>(numInputChannels) : 1.0f);
            monoBuffer[s] = mono;
            peak = std::max(peak, std::abs(mono));
        }

        levelAtomic.store(peak, std::memory_order_relaxed);

        if (!running)
            return;

        audiotomidi::BeatDetector::Params detParams;
        detParams.sensitivity = static_cast<float>(sensitivitySlider.getValue());
        detParams.minGapMs = static_cast<float>(minGapSlider.getValue());
        detParams.focusLow = focusLowToggle.getToggleState();

        audiotomidi::BeatDetector::TriggerBuffer triggers;
        detector.processBlock(monoBuffer.get(), numSamples, detParams, triggers);

        if (triggers.count > 0)
            triggerAtomic.store(true, std::memory_order_relaxed);

        audiotomidi::MidiEngineParams midiParams;
        midiParams.noteNumber = static_cast<int>(noteSlider.getValue());
        midiParams.midiChannel = static_cast<int>(channelSlider.getValue());
        midiParams.noteLengthMs = static_cast<int>(noteLenSlider.getValue());
        midiParams.velocityMode = velocityModeBox.getSelectedId() == 1 ? audiotomidi::VelocityMode::Fixed : audiotomidi::VelocityMode::Dynamic;
        midiParams.fixedVelocity = static_cast<int>(velocitySlider.getValue());

        juce::MidiBuffer midi;
        midiEngine.process(triggers, midi, numSamples, midiParams);

        if (midiOutput != nullptr && !midi.isEmpty())
            midiOutput->sendBlockOfMessages(midi, juce::Time::getMillisecondCounterHiRes(), currentSampleRate);
    }

    void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) override
    {
        if (comboBoxThatHasChanged == &midiOutputBox)
            openSelectedMidiDevice();
    }

    void timerCallback() override
    {
        levelMeter.setLevel(levelAtomic.load(std::memory_order_relaxed));
        triggerLed.setTriggered(triggerAtomic.exchange(false, std::memory_order_relaxed));
    }

    void refreshDevices()
    {
        midiOutputBox.clear(juce::dontSendNotification);

        const auto devices = juce::MidiOutput::getAvailableDevices();
        int id = 1;
        int selectedId = 0;

        for (const auto& d : devices)
        {
            midiOutputBox.addItem(d.name, id);
            if (d.name == settings.midiOutputName)
                selectedId = id;
            ++id;
        }

        if (midiOutputBox.getNumItems() > 0)
            midiOutputBox.setSelectedId(selectedId > 0 ? selectedId : 1, juce::sendNotification);
    }

    void openSelectedMidiDevice()
    {
        midiOutput.reset();
        const auto idx = midiOutputBox.getSelectedItemIndex();
        if (idx < 0)
            return;

        const auto devices = juce::MidiOutput::getAvailableDevices();
        if (idx >= static_cast<int>(devices.size()))
            return;

        midiOutput = juce::MidiOutput::openDevice(devices[static_cast<size_t>(idx)].identifier);
    }

    void restoreSettings(juce::PropertiesFile& props)
    {
        settings.audioState = props.getValue("audioState");
        settings.midiOutputName = props.getValue("midiOutput", {});
        settings.sensitivity = props.getDoubleValue("sensitivity", 60.0);
        settings.minGapMs = props.getDoubleValue("minGapMs", 120.0);
        settings.noteNumber = props.getIntValue("noteNumber", 36);
        settings.midiChannel = props.getIntValue("midiChannel", 1);
        settings.noteLengthMs = props.getIntValue("noteLengthMs", 30);
        settings.fixedVelocity = props.getIntValue("fixedVelocity", 100);
        settings.velocityModeId = props.getIntValue("velocityModeId", 1);
        settings.focusLow = props.getBoolValue("focusLow", true);
        settings.running = props.getBoolValue("running", true);
    }

    void saveSettings(juce::PropertiesFile& props)
    {
        if (auto stateXml = deviceManager.createStateXml())
            props.setValue("audioState", stateXml->toString());

        props.setValue("midiOutput", midiOutputBox.getText());
        props.setValue("sensitivity", sensitivitySlider.getValue());
        props.setValue("minGapMs", minGapSlider.getValue());
        props.setValue("noteNumber", static_cast<int>(noteSlider.getValue()));
        props.setValue("midiChannel", static_cast<int>(channelSlider.getValue()));
        props.setValue("noteLengthMs", static_cast<int>(noteLenSlider.getValue()));
        props.setValue("fixedVelocity", static_cast<int>(velocitySlider.getValue()));
        props.setValue("velocityModeId", velocityModeBox.getSelectedId());
        props.setValue("focusLow", focusLowToggle.getToggleState());
        props.setValue("running", running);
        props.saveIfNeeded();
    }

    struct SavedSettings
    {
        juce::String audioState;
        juce::String midiOutputName;
        double sensitivity = 60.0;
        double minGapMs = 120.0;
        int noteNumber = 36;
        int midiChannel = 1;
        int noteLengthMs = 30;
        int fixedVelocity = 100;
        int velocityModeId = 1;
        bool focusLow = true;
        bool running = true;
    } settings;

    juce::AudioDeviceManager deviceManager;
    juce::AudioDeviceSelectorComponent audioSelector;

    juce::Label midiOutLabel;
    juce::ComboBox midiOutputBox;
    juce::TextButton startStopButton;
    juce::TextButton refreshButton;

    juce::Label levelLabel;
    LevelMeter levelMeter;
    juce::Label triggerLabel;
    TriggerLed triggerLed;

    juce::Slider sensitivitySlider, minGapSlider, noteSlider, channelSlider, noteLenSlider, velocitySlider;
    juce::Label sensitivityLabel, minGapLabel, noteLabel, channelLabel, noteLenLabel, velocityLabel;
    juce::ComboBox velocityModeBox;
    juce::ToggleButton focusLowToggle;

    juce::HeapBlock<float> monoBuffer;
    int monoBufferSize = 0;
    double currentSampleRate = 44100.0;

    audiotomidi::BeatDetector detector;
    audiotomidi::MidiEngine midiEngine;
    std::unique_ptr<juce::MidiOutput> midiOutput;

    std::atomic<float> levelAtomic { 0.0f };
    std::atomic<bool> triggerAtomic { false };

    bool running = true;

    juce::ApplicationProperties appProperties;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StandaloneMainComponent)
};

class AudioToMidiBeatApplication : public juce::JUCEApplication
{
public:
    const juce::String getApplicationName() override { return "AudioToMidiBeat"; }
    const juce::String getApplicationVersion() override { return "1.0.0"; }

    void initialise(const juce::String&) override
    {
        mainWindow = std::make_unique<MainWindow>(getApplicationName());
    }

    void shutdown() override
    {
        mainWindow = nullptr;
    }

private:
    class MainWindow : public juce::DocumentWindow
    {
    public:
        explicit MainWindow(const juce::String& name)
            : juce::DocumentWindow(name,
                                   juce::Desktop::getInstance().getDefaultLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId),
                                   juce::DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar(true);
            auto content = std::make_unique<StandaloneMainComponent>();
            setContentOwned(content.release(), true);
            setResizable(true, true);
            centreWithSize(getWidth(), getHeight());
            setVisible(true);
        }

        void closeButtonPressed() override
        {
            juce::JUCEApplication::getInstance()->systemRequestedQuit();
        }
    };

    std::unique_ptr<MainWindow> mainWindow;
};

} // namespace

START_JUCE_APPLICATION(AudioToMidiBeatApplication)

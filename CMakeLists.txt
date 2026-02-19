cmake_minimum_required(VERSION 3.22)
project(AudioToMidiBeat VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  JUCE
  GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
  GIT_TAG 8.0.6
)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(AudioToMidiBeat
    COMPANY_NAME "AudioToMidiBeat"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT TRUE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE AtMB
    PLUGIN_CODE AtmB
    FORMATS VST3
    PRODUCT_NAME "AudioToMidiBeat")

target_sources(AudioToMidiBeat PRIVATE
    src/PluginProcessor.cpp
    src/PluginProcessor.h
    src/PluginEditor.cpp
    src/PluginEditor.h
    src/BeatDetector.cpp
    src/BeatDetector.h
    src/MidiEngine.cpp
    src/MidiEngine.h)

target_compile_definitions(AudioToMidiBeat
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(AudioToMidiBeat
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

juce_add_gui_app(AudioToMidiBeatApp
    PRODUCT_NAME "AudioToMidiBeat")

target_sources(AudioToMidiBeatApp PRIVATE
    src/Main.cpp
    src/BeatDetector.cpp
    src/BeatDetector.h
    src/MidiEngine.cpp
    src/MidiEngine.h)

target_compile_definitions(AudioToMidiBeatApp PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0)

target_link_libraries(AudioToMidiBeatApp PRIVATE
    juce::juce_audio_utils
    juce::juce_gui_extra
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags)

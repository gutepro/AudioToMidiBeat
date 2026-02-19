# AudioToMidiBeat

AudioToMidiBeat is a JUCE-based C++17 project that converts live audio input (microphone or line-in) into MIDI trigger events in real time.

It builds two products on Windows and macOS:
- Standalone desktop application
- VST3 plugin

## Core Trigger Engine

Audio processing path:
- Reads incoming audio buffer
- Absolute-value rectification
- One-pole envelope follower
- Optional 180Hz low-pass focus (`FocusLow`)
- Adaptive threshold using slow noise-floor tracking
- Refractory period via `MinGapMs`

Trigger condition:
- Envelope crosses threshold upward
- Time since last trigger >= `MinGapMs`

On trigger:
- MIDI Note On
- MIDI Note Off after `NoteLengthMs`
- Velocity mode: Fixed or Dynamic

## Parameters (Automatable in VST3)

- Sensitivity (0-100), default `60`
- MinGapMs (50-300), default `120`
- NoteNumber (0-127), default `36`
- MidiChannel (1-16), default `1`
- NoteLengthMs (10-120), default `30`
- VelocityMode (`Fixed` / `Dynamic`), default `Fixed`
- FixedVelocity (0-127), default `100`
- FocusLow (`On`/`Off`), default `On`

## Project Structure

```text
AudioToMidiBeat/
├── CMakeLists.txt
├── README.md
├── LICENSE
├── .gitignore
├── src/
│   ├── Main.cpp
│   ├── PluginProcessor.h
│   ├── PluginProcessor.cpp
│   ├── PluginEditor.h
│   ├── PluginEditor.cpp
│   ├── BeatDetector.h
│   ├── BeatDetector.cpp
│   ├── MidiEngine.h
│   ├── MidiEngine.cpp
├── packaging/
│   ├── windows_installer.iss
│   ├── mac_dmg.sh
└── .github/workflows/
    ├── build-windows.yml
    ├── build-macos.yml
```

## Build Instructions

### Windows

Requirements:
- Visual Studio 2022 (Desktop C++)
- CMake 3.22+

Build:
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

Outputs include:
- `AudioToMidiBeatApp.exe`
- `AudioToMidiBeat.vst3`

Installer:
```bash
iscc packaging\\windows_installer.iss
```

### macOS

Requirements:
- Xcode 15+
- CMake 3.22+

Build:
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

Outputs include:
- `AudioToMidiBeatApp.app`
- `AudioToMidiBeat.vst3`

DMG packaging:
```bash
chmod +x packaging/mac_dmg.sh
./packaging/mac_dmg.sh build dist
```

## Installation

### Windows
From GitHub Actions artifacts:
1. Open the repository `Actions` tab.
2. Open a successful `build-windows` run.
3. Download artifact `AudioToMidiBeat-Windows-Installer`.
4. Extract and run `AudioToMidiBeat-Setup.exe`.

Install result:
- Standalone app installs to `C:\Program Files\AudioToMidiBeat`
- Start Menu shortcut is created
- Uninstall entry is created automatically
- VST3 installs to `C:\Program Files\Common Files\VST3\AudioToMidiBeat.vst3` when included

### macOS
From GitHub Actions artifacts:
1. Open the repository `Actions` tab.
2. Open a successful `build-macos` run.
3. Download artifact `AudioToMidiBeat-macOS-DMG`.
4. Open `AudioToMidiBeat.dmg`.
5. Drag `AudioToMidiBeatApp.app` into `Applications`.

Notes:
- MVP CI currently produces unsigned packages when signing secrets are not configured.
- Workflows include placeholders for future Windows code signing and macOS notarization.

## Standalone Usage

- Select audio input device from Audio Device panel
- Select MIDI output device from MIDI Output drop-down
- Use `Start/Stop` to enable/disable trigger generation
- Use `Refresh Devices` after connecting new interfaces
- Last-used configuration is saved via local app settings

## VST3 Usage

- Insert plugin on an audio track in host
- Feed audio input into plugin
- Route plugin MIDI output to destination instrument or controller
- All trigger parameters are automatable

## Routing MIDI to GrandMA (Example)

1. In standalone app, set MIDI Output to your virtual/physical MIDI port used by GrandMA.
2. Keep defaults for first test:
   - NoteNumber: `36`
   - MidiChannel: `1`
3. In GrandMA, map incoming Note 36 on Channel 1 to your target executor/cue action.
4. Adjust `Sensitivity` and `MinGapMs` for stable triggering per source material.

## Default MIDI Mapping Example

- Note: `36`
- Channel: `1`
- Velocity mode: `Fixed`
- Velocity: `100`

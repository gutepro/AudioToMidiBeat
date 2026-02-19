; AudioToMidiBeat Windows Installer Script
#define AppName "AudioToMidiBeat"
#define AppVersion "1.0.0"
#define Publisher "AudioToMidiBeat"
#define URL "https://github.com/gutepro/AudioToMidiBeat"

#ifndef MyAppExe
  #define MyAppExe "build\Release\AudioToMidiBeatApp.exe"
#endif

#ifndef MyVST3
  #define MyVST3 "build\AudioToMidiBeat_artefacts\Release\VST3\AudioToMidiBeat.vst3"
#endif

[Setup]
AppId={{3C4D2D85-C913-4D06-B338-1E9260AC47E1}
AppName={#AppName}
AppVersion={#AppVersion}
AppPublisher={#Publisher}
AppPublisherURL={#URL}
DefaultDirName={autopf}\{#AppName}
DefaultGroupName={#AppName}
Compression=lzma
SolidCompression=yes
OutputDir=.
OutputBaseFilename=AudioToMidiBeatInstaller
WizardStyle=modern
ArchitecturesInstallIn64BitMode=x64

[Files]
Source: "{#MyAppExe}"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#MyVST3}"; DestDir: "{cf}\VST3"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\{#AppName}"; Filename: "{app}\AudioToMidiBeatApp.exe"

[Run]
Filename: "{app}\AudioToMidiBeatApp.exe"; Description: "Launch {#AppName}"; Flags: nowait postinstall skipifsilent

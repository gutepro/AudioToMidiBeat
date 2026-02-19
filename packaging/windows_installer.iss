; AudioToMidiBeat Windows Installer Script
#define AppName "AudioToMidiBeat"
#define AppVersion "0.1.0"
#define Publisher "AudioToMidiBeat"
#define URL "https://github.com/gutepro/AudioToMidiBeat"

#ifndef OutputDir
  #define OutputDir "dist\\windows"
#endif

#ifndef MyAppExe
  #define MyAppExe "dist\\windows\\app\\AudioToMidiBeatApp.exe"
#endif

[Setup]
AppId={{3C4D2D85-C913-4D06-B338-1E9260AC47E1}
AppName={#AppName}
AppVersion={#AppVersion}
AppPublisher={#Publisher}
AppPublisherURL={#URL}
AppSupportURL={#URL}
DefaultDirName={pf}\AudioToMidiBeat
DefaultGroupName={#AppName}
DisableProgramGroupPage=no
Compression=lzma
SolidCompression=yes
OutputDir={#OutputDir}
OutputBaseFilename=AudioToMidiBeat-Setup
WizardStyle=modern
ArchitecturesInstallIn64BitMode=x64
UninstallDisplayIcon={app}\AudioToMidiBeatApp.exe

[Files]
Source: "{#MyAppExe}"; DestDir: "{app}"; Flags: ignoreversion
#ifdef MyVST3
Source: "{#MyVST3}\*"; DestDir: "{commoncf}\VST3\AudioToMidiBeat.vst3"; Flags: ignoreversion recursesubdirs createallsubdirs skipifsourcedoesntexist
#endif

[Icons]
Name: "{group}\AudioToMidiBeat"; Filename: "{app}\AudioToMidiBeatApp.exe"
Name: "{group}\Uninstall AudioToMidiBeat"; Filename: "{uninstallexe}"

[Run]
Filename: "{app}\AudioToMidiBeatApp.exe"; Description: "Launch AudioToMidiBeat"; Flags: nowait postinstall skipifsilent

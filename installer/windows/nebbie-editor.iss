; Nebbie Editor — Windows installer (Inno Setup)
; Build: iscc /DAppVersion=0.1.0 /DStagingDir=C:\path\dist\windows-staging nebbie-editor.iss

#ifndef AppVersion
  #define AppVersion "0.1.0"
#endif

#ifndef StagingDir
  #define StagingDir "..\..\dist\windows-staging"
#endif

#define AppName "Nebbie Editor"
#define AppPublisher "Nebbie Editor"
#define AppURL "https://github.com/wizardmorgan/nebbie-editor"
#define AppExeName "nebbieedit.exe"
#define CliExeName "nebbiedit.exe"

[Setup]
AppId={{A8F4E2B1-9C3D-4E5F-A6B7-1D2E3F4A5B6C}
AppName={#AppName}
AppVersion={#AppVersion}
AppVerName={#AppName} {#AppVersion}
AppPublisher={#AppPublisher}
AppPublisherURL={#AppURL}
AppSupportURL={#AppURL}
AppUpdatesURL={#AppURL}
DefaultDirName={autopf}\{#AppName}
DefaultGroupName={#AppName}
AllowNoIcons=yes
LicenseFile=
OutputDir=..\..\dist
OutputBaseFilename=nebbie-editor_{#AppVersion}_windows_setup
SetupIconFile={#StagingDir}\nebbieedit.ico
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
PrivilegesRequired=lowest
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
UninstallDisplayIcon={app}\{#AppExeName}

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "italian"; MessagesFile: "compiler:Languages\Italian.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "addpath"; Description: "Add install folder to PATH (for nebbiedit CLI)"; GroupDescription: "Optional:"; Flags: unchecked

[Files]
Source: "{#StagingDir}\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\{#AppName}"; Filename: "{app}\{#AppExeName}"; IconFilename: "{app}\nebbieedit.ico"
Name: "{group}\Nebbie CLI (nebbiedit)"; Filename: "{app}\{#CliExeName}"; IconFilename: "{app}\nebbieedit.ico"
Name: "{autodesktop}\{#AppName}"; Filename: "{app}\{#AppExeName}"; IconFilename: "{app}\nebbieedit.ico"; Tasks: desktopicon

[Run]
Filename: "{app}\{#AppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(AppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

[Registry]
Root: HKCU; Subkey: "Environment"; ValueType: expandsz; ValueName: "Path"; ValueData: "{olddata};{app}"; Tasks: addpath; Check: NeedsAddPath(ExpandConstant('{app}'))

[Code]
function NeedsAddPath(Param: string): Boolean;
var
  OrigPath: string;
begin
  if not RegQueryStringValue(HKEY_CURRENT_USER, 'Environment', 'Path', OrigPath) then
  begin
    Result := True;
    exit;
  end;
  Result := Pos(';' + Param + ';', ';' + OrigPath + ';') = 0;
end;

; Wyrmhold installer (Inno Setup 6), in English, Japanese and Spanish.
; Built by tools/make_installer.ps1, which passes /DAppVersion.
; Author: Gabriel Castiglia - 2026-11-10
#ifndef AppVersion
  #define AppVersion "2.0.0"
#endif

[Setup]
AppId={{0B42F4DD-81B9-4012-A29C-7DAE09A1DA5A}
AppName=Wyrmhold
AppVersion={#AppVersion}
AppVerName=Wyrmhold {#AppVersion}
AppPublisher=Gabriel Castiglia
AppCopyright=© 2026 Gabriel Castiglia
LicenseFile=..\LICENSE
; per user by default (no administrator needed); the dialog offers all users
DefaultDirName={autopf}\Wyrmhold
DefaultGroupName=Wyrmhold
DisableProgramGroupPage=yes
PrivilegesRequired=lowest
PrivilegesRequiredOverridesAllowed=dialog
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
MinVersion=10.0.17763
OutputDir=..\out
OutputBaseFilename=Wyrmhold-Setup-{#AppVersion}
SetupIconFile=..\assets\app.ico
UninstallDisplayIcon={app}\wyrmhold.exe
UninstallDisplayName=Wyrmhold
WizardStyle=modern
WizardImageFile=wizard.bmp
WizardSmallImageFile=wizard-small.bmp
ShowLanguageDialog=yes
Compression=lzma2/ultra64
SolidCompression=yes
CloseApplications=yes
VersionInfoVersion={#AppVersion}
VersionInfoProductName=Wyrmhold
VersionInfoDescription=Wyrmhold Setup

[Languages]
Name: "en"; MessagesFile: "compiler:Default.isl"
Name: "ja"; MessagesFile: "compiler:Languages\Japanese.isl"
Name: "es"; MessagesFile: "compiler:Languages\Spanish.isl"

[CustomMessages]
en.Guide=Wyrmhold user guide
ja.Guide=Wyrmhold ユーザーガイド
es.Guide=Guía de uso de Wyrmhold
en.Tagline=Strong passwords, passphrases and PINs, made on this computer and stored nowhere.
ja.Tagline=強力なパスワード・パスフレーズ・PIN を、このコンピューターで作り、どこにも保存しません。
es.Tagline=Contraseñas, frases y PIN fuertes, hechos en esta computadora y sin guardarse en ningún lado.
en.RemoveSettings=Also delete Wyrmhold's settings (language, theme, last options)?
ja.RemoveSettings=Wyrmhold の設定（言語・テーマ・最後のオプション）も削除しますか？
es.RemoveSettings=¿Borrar también la configuración de Wyrmhold (idioma, tema, últimas opciones)?

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "..\build\wyrmhold.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\LICENSE"; DestDir: "{app}"; DestName: "LICENSE.txt"; Flags: ignoreversion
Source: "..\docs\wyrmhold_guide_en.pdf"; DestDir: "{app}\docs"; Flags: ignoreversion
Source: "..\docs\wyrmhold_guide_jp.pdf"; DestDir: "{app}\docs"; Flags: ignoreversion
Source: "..\docs\wyrmhold_guide_es.pdf"; DestDir: "{app}\docs"; Flags: ignoreversion

[Icons]
Name: "{autoprograms}\Wyrmhold"; Filename: "{app}\wyrmhold.exe"; Comment: "{cm:Tagline}"
Name: "{autoprograms}\{cm:Guide}"; Filename: "{app}\docs\wyrmhold_guide_en.pdf"; Languages: en
Name: "{autoprograms}\{cm:Guide}"; Filename: "{app}\docs\wyrmhold_guide_jp.pdf"; Languages: ja
Name: "{autoprograms}\{cm:Guide}"; Filename: "{app}\docs\wyrmhold_guide_es.pdf"; Languages: es
Name: "{autodesktop}\Wyrmhold"; Filename: "{app}\wyrmhold.exe"; Comment: "{cm:Tagline}"; Tasks: desktopicon

[Run]
; the program starts in the language chosen for the setup
Filename: "{app}\wyrmhold.exe"; Parameters: "/lang:{code:AppLang}"; Description: "{cm:LaunchProgram,Wyrmhold}"; Flags: nowait postinstall skipifsilent

[Code]
// The setup's language -> the program's /lang: value (the program remembers it).
function AppLang(Param: String): String;
begin
  if ActiveLanguage = 'ja' then Result := 'jp'
  else if ActiveLanguage = 'es' then Result := 'es'
  else Result := 'en';
end;

// Settings live in %APPDATA%\Wyrmhold (wyrmhold.ini). Ask before deleting them, and never in
// a silent uninstall.
procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
var
  Dir: String;
begin
  if CurUninstallStep = usPostUninstall then
  begin
    Dir := ExpandConstant('{userappdata}\Wyrmhold');
    if DirExists(Dir) and not UninstallSilent then
      if MsgBox(CustomMessage('RemoveSettings'), mbConfirmation, MB_YESNO or MB_DEFBUTTON2) = IDYES then
        DelTree(Dir, True, True, True);
  end;
end;

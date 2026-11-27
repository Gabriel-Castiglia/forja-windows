; Forja installer (Inno Setup 6), in English, Japanese and Spanish.
; Built by tools/make_installer.ps1, which passes /DAppVersion.
; Copyright (C) 2026 Gabriel Castiglia. Licensed under the GNU GPL, version 2 only.
; Date: November 10, 2026
; SPDX-License-Identifier: GPL-2.0-only
; AppVersion is always passed by tools\make_installer.ps1, which reads it from src\version.h.
#ifndef AppVersion
  #define AppVersion "0.0.0"
#endif

[Setup]
AppId={{AA7E8B36-E890-4F08-8A5E-C161BE196A52}
AppName=Forja
AppVersion={#AppVersion}
AppVerName=Forja {#AppVersion}
AppPublisher=Gabriel Castiglia
AppCopyright=© 2026 Gabriel Castiglia
LicenseFile=..\LICENSE
; per user by default (no administrator needed); the dialog offers all users
DefaultDirName={autopf}\Forja
DefaultGroupName=Forja
DisableProgramGroupPage=yes
PrivilegesRequired=lowest
PrivilegesRequiredOverridesAllowed=dialog
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
MinVersion=10.0.17763
OutputDir=..\out
OutputBaseFilename=Forja-Setup-{#AppVersion}
SetupIconFile=..\assets\app.ico
UninstallDisplayIcon={app}\forja.exe
UninstallDisplayName=Forja
WizardStyle=modern
WizardImageFile=wizard.bmp
WizardSmallImageFile=wizard-small.bmp
ShowLanguageDialog=yes
Compression=lzma2/ultra64
SolidCompression=yes
CloseApplications=yes
VersionInfoVersion={#AppVersion}
VersionInfoProductName=Forja
VersionInfoDescription=Forja Setup

[Languages]
Name: "en"; MessagesFile: "compiler:Default.isl"
Name: "ja"; MessagesFile: "compiler:Languages\Japanese.isl"
Name: "es"; MessagesFile: "compiler:Languages\Spanish.isl"

[CustomMessages]
en.Guide=Forja user guide
ja.Guide=Forja ユーザーガイド
es.Guide=Guía de uso de Forja
en.Tagline=Strong passwords, passphrases and PINs, made on this computer and stored nowhere.
ja.Tagline=強力なパスワード・パスフレーズ・PIN を、このコンピューターで作り、どこにも保存しません。
es.Tagline=Contraseñas, frases y PIN fuertes, creados en este equipo y sin guardarse en ninguna parte.
en.RemoveSettings=Also delete Forja's settings (language, theme, last options)?
ja.RemoveSettings=Forja の設定（言語・テーマ・最後のオプション）も削除しますか？
es.RemoveSettings=¿Borrar también la configuración de Forja (idioma, tema, últimas opciones)?
en.WyrmholdLeft=Wyrmhold, the former name of Forja, could not be uninstalled (code %1). Both are installed now: you can remove Wyrmhold from Settings > Apps.
ja.WyrmholdLeft=Forja の旧名 Wyrmhold をアンインストールできませんでした（コード %1）。現在は両方がインストールされています。Wyrmhold は［設定］>［アプリ］から削除できます。
es.WyrmholdLeft=No se pudo desinstalar Wyrmhold, el nombre anterior de Forja (código %1). Ahora están instalados los dos: puedes quitar Wyrmhold desde Configuración > Aplicaciones.

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"

[Files]
Source: "..\build\forja.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\LICENSE"; DestDir: "{app}"; DestName: "LICENSE.txt"; Flags: ignoreversion
Source: "..\docs\forja_guide_en.pdf"; DestDir: "{app}\docs"; Flags: ignoreversion
Source: "..\docs\forja_guide_jp.pdf"; DestDir: "{app}\docs"; Flags: ignoreversion
Source: "..\docs\forja_guide_es.pdf"; DestDir: "{app}\docs"; Flags: ignoreversion

[Icons]
Name: "{autoprograms}\Forja"; Filename: "{app}\forja.exe"; Comment: "{cm:Tagline}"
Name: "{autoprograms}\{cm:Guide}"; Filename: "{app}\docs\forja_guide_en.pdf"; Languages: en
Name: "{autoprograms}\{cm:Guide}"; Filename: "{app}\docs\forja_guide_jp.pdf"; Languages: ja
Name: "{autoprograms}\{cm:Guide}"; Filename: "{app}\docs\forja_guide_es.pdf"; Languages: es
Name: "{autodesktop}\Forja"; Filename: "{app}\forja.exe"; Comment: "{cm:Tagline}"; Tasks: desktopicon

[Run]
; the program starts in the language chosen for the setup
Filename: "{app}\forja.exe"; Parameters: "/lang:{code:AppLang}"; Description: "{cm:LaunchProgram,Forja}"; Flags: nowait postinstall skipifsilent
; Help -> Check for updates installs in silence and asks for the program to come back
Filename: "{app}\forja.exe"; Parameters: "/updated"; Flags: nowait runasoriginaluser; Check: WhRelaunch

[Code]
// Forja was called Wyrmhold up to 2.1 (another AppId): that install is removed in silence before the new files
// are copied; its settings (%APPDATA%\Wyrmhold\wyrmhold.ini) come over when Forja first starts.
const
  WyrmholdKey = 'Software\Microsoft\Windows\CurrentVersion\Uninstall\{0B42F4DD-81B9-4012-A29C-7DAE09A1DA5A}_is1';

// If it cannot be removed (an install for all users while this setup runs per user, an
// elevation prompt refused...) the setup goes on and says so, instead of leaving both in silence.
procedure RemoveWyrmhold();
var
  S: String;
  Rc: Integer;
begin
  if RegQueryStringValue(HKCU, WyrmholdKey, 'UninstallString', S) or
     RegQueryStringValue(HKLM, WyrmholdKey, 'UninstallString', S) then
  begin
    S := RemoveQuotes(S);
    if FileExists(S) then
    begin
      if not Exec(S, '/VERYSILENT /SUPPRESSMSGBOXES /NORESTART', '', SW_HIDE, ewWaitUntilTerminated, Rc) then
        Rc := -1;
      if (Rc <> 0) and (RegValueExists(HKCU, WyrmholdKey, 'UninstallString') or
                        RegValueExists(HKLM, WyrmholdKey, 'UninstallString')) then
        SuppressibleMsgBox(FmtMessage(CustomMessage('WyrmholdLeft'), [IntToStr(Rc)]), mbError, MB_OK, IDOK);
    end;
  end;
end;

procedure CurStepChanged(CurStep: TSetupStep);
begin
  if CurStep = ssInstall then RemoveWyrmhold();
end;

// Started by Forja itself (the self-installing update, src/updui.c): /WHWAITPID=<pid> waits for
// that process to finish closing before any file is replaced; /WHRELAUNCH=1 starts the program
// again when the files are in place. A silent setup keeps the previous install folder, mode,
// language and tasks (UsePrevious* default to yes).
function OpenProcess(Access: Cardinal; Inherit: Boolean; Pid: Cardinal): Cardinal;
  external 'OpenProcess@kernel32.dll stdcall';
function WaitForSingleObject(H: Cardinal; Ms: Cardinal): Cardinal;
  external 'WaitForSingleObject@kernel32.dll stdcall';
function CloseHandle(H: Cardinal): Boolean;
  external 'CloseHandle@kernel32.dll stdcall';

function InitializeSetup(): Boolean;
var
  Pid: Integer;
  H: Cardinal;
begin
  Pid := StrToIntDef(ExpandConstant('{param:WHWAITPID|0}'), 0);
  if Pid > 0 then
  begin
    H := OpenProcess($00100000, False, Pid);   // SYNCHRONIZE
    if H <> 0 then
    begin
      WaitForSingleObject(H, 60000);
      CloseHandle(H);
    end;
  end;
  Result := True;
end;

function WhRelaunch(): Boolean;
begin
  Result := ExpandConstant('{param:WHRELAUNCH|0}') = '1';
end;

// The setup's language -> the program's /lang: value (the program remembers it).
function AppLang(Param: String): String;
begin
  if ActiveLanguage = 'ja' then Result := 'jp'
  else if ActiveLanguage = 'es' then Result := 'es'
  else Result := 'en';
end;

// Settings live in %APPDATA%\Forja (forja.ini); %APPDATA%\Wyrmhold may still hold the ones of
// the former name, which Forja read once and left there. Ask before deleting them (one question
// for both), and never in a silent uninstall.
procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
var
  Dir, Old: String;
begin
  if CurUninstallStep = usPostUninstall then
  begin
    Dir := ExpandConstant('{userappdata}\Forja');
    Old := ExpandConstant('{userappdata}\Wyrmhold');
    if (DirExists(Dir) or DirExists(Old)) and not UninstallSilent then
      if MsgBox(CustomMessage('RemoveSettings'), mbConfirmation, MB_YESNO or MB_DEFBUTTON2) = IDYES then
      begin
        if DirExists(Dir) then DelTree(Dir, True, True, True);
        if DirExists(Old) then DelTree(Old, True, True, True);
      end;
  end;
end;

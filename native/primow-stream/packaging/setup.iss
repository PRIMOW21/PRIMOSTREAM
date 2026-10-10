#ifndef Stage
  #error Stage must point to the staged runtime
#endif
[Setup]
AppId=PrimowStream
; Keep the upgrade identity; the public product name has changed.
AppName=NERON STREAM PROJECT
AppVersion=0.2.0
DefaultDirName={autopf}\NERON STREAM PROJECT
DefaultGroupName=NERON STREAM PROJECT
UsePreviousGroup=no
OutputDir={#Output}
OutputBaseFilename=NERON-STREAM-PROJECT-Setup
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
MinVersion=10.0.17763
Compression=lzma2
SolidCompression=yes
PrivilegesRequired=admin
UninstallDisplayIcon={app}\bin\64bit\neron-stream-project.exe
[Files]
Source: "{#Stage}\*"; DestDir: "{app}"; Flags: recursesubdirs createallsubdirs ignoreversion
[Icons]
Name: "{group}\NERON STREAM PROJECT"; Filename: "{app}\bin\64bit\neron-stream-project.exe"; WorkingDir: "{app}\bin\64bit"
Name: "{autodesktop}\NERON STREAM PROJECT"; Filename: "{app}\bin\64bit\neron-stream-project.exe"; WorkingDir: "{app}\bin\64bit"
[InstallDelete]
Type: files; Name: "{group}\PRIMOW STREAM.lnk"
Type: files; Name: "{userprograms}\PRIMOW STREAM\PRIMOW STREAM.lnk"
Type: files; Name: "{commonprograms}\PRIMOW STREAM\PRIMOW STREAM.lnk"
Type: files; Name: "{autodesktop}\PRIMOW STREAM.lnk"
Type: files; Name: "{app}\bin\64bit\primow-stream.exe"
[Run]
Filename: "{app}\bin\64bit\neron-stream-project.exe"; Description: "Abrir NERON STREAM PROJECT"; Flags: postinstall nowait skipifsilent

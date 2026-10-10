#ifndef Stage
  #error Stage must point to the staged runtime
#endif
[Setup]
AppId=PrimowStream
AppName=PRIMOW STREAM
AppVersion=0.1.0
DefaultDirName={autopf}\PRIMOW STREAM
DefaultGroupName=PRIMOW STREAM
OutputDir={#Output}
OutputBaseFilename=PRIMOW-STREAM-Setup
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
MinVersion=10.0.17763
Compression=lzma2
SolidCompression=yes
PrivilegesRequired=admin
UninstallDisplayIcon={app}\bin\64bit\primow-stream.exe
[Files]
Source: "{#Stage}\*"; DestDir: "{app}"; Flags: recursesubdirs createallsubdirs ignoreversion
[Icons]
Name: "{group}\PRIMOW STREAM"; Filename: "{app}\bin\64bit\primow-stream.exe"; WorkingDir: "{app}\bin\64bit"
Name: "{autodesktop}\PRIMOW STREAM"; Filename: "{app}\bin\64bit\primow-stream.exe"; WorkingDir: "{app}\bin\64bit"
[Run]
Filename: "{app}\bin\64bit\primow-stream.exe"; Description: "Abrir PRIMOW STREAM"; Flags: postinstall nowait skipifsilent

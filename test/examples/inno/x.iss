#define app_copyright "Copyright 1999, app corporation"

[Setup]
AppName=MyApp
AppCopyright={#app_copyright}

[Files]
Source: "app.exe"; DestDir: "{tmp}"
Source: 'helper.exe'; DestDir: '{tmp}'

[Code]
function ShouldInstallComCtlUpdate: Boolean;
begin
  Result := False;
end;

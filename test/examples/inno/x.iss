#define app_copyright "Copyright 1999, app corporation"

[Setup]
AppName=MyApp
AppCopyright={#app_copyright}
WizardSmallImageFile=WizardSmallImageFile.bmp
OnlyBelowVersion=6.01

[Files]
Source: "app.exe"; DestDir: "{tmp}"; OnlyBelowVersion: 6.01
Source: 'helper.exe'; DestDir: '{tmp}'

[Code]
function ShouldInstallComCtlUpdate: Boolean;
begin
  Result := False;
end;

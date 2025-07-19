@echo off
setlocal

echo Select a location to place shortcuts:

: Prompt user for shortcut directory
set "psCommand="(new-object -COM 'Shell.Application')^
.BrowseForFolder(0,'Please choose a folder.',0,0).self.path""

for /f "usebackq delims=" %%I in (`powershell %psCommand%`) do set "folder=%%I"

setlocal enabledelayedexpansion
echo Placing shortcuts in !folder!
endlocal

: Set the CANTOOLS directory environment variable
setx ZRE_CANTOOLS_DIR %~dp0%

: Create batch script shortcuts
powershell "$s=(New-Object -COM WScript.Shell).CreateShortcut('%folder%\cross-bms.lnk');$s.TargetPath='%~dp0%/bin/cross-bms';$s.Save()"
powershell "$s=(New-Object -COM WScript.Shell).CreateShortcut('%folder%\cross-can.lnk');$s.TargetPath='%~dp0%/bin/cross-can';$s.Save()"
powershell "$s=(New-Object -COM WScript.Shell).CreateShortcut('%folder%\glory-bms-charger.lnk');$s.TargetPath='%~dp0%/bin/glory-bms-charger';$s.Save()"
powershell "$s=(New-Object -COM WScript.Shell).CreateShortcut('%folder%\glory-bms-vehicle.lnk');$s.TargetPath='%~dp0%/bin/glory-bms-vehicle';$s.Save()"
powershell "$s=(New-Object -COM WScript.Shell).CreateShortcut('%folder%\glory-can-charger.lnk');$s.TargetPath='%~dp0%/bin/glory-can-charger';$s.Save()"
powershell "$s=(New-Object -COM WScript.Shell).CreateShortcut('%folder%\glory-can-vehicle.lnk');$s.TargetPath='%~dp0%/bin/glory-can-vehicle';$s.Save()"
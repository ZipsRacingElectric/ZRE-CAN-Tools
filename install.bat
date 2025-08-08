@echo off
setlocal

:: Set the CANTOOLS directory environment variable
setx ZRE_CANTOOLS_DIR %~dp0%

:: Create the start menu shortcut directory if it doesn't exist
set "FOLDER=%APPDATA%\Microsoft\Windows\Start Menu\Programs\ZRE"
if not exist "%FOLDER%" mkdir "%FOLDER%"

:: Create batch script shortcuts
powershell "$s=(New-Object -COM WScript.Shell).CreateShortcut('%FOLDER%\cross-bms.lnk');$s.TargetPath='%~dp0%/bin/cross-bms';$s.Save()"
powershell "$s=(New-Object -COM WScript.Shell).CreateShortcut('%FOLDER%\cross-can.lnk');$s.TargetPath='%~dp0%/bin/cross-can';$s.Save()"
powershell "$s=(New-Object -COM WScript.Shell).CreateShortcut('%FOLDER%\glory-bms-charger.lnk');$s.TargetPath='%~dp0%/bin/glory-bms-charger';$s.Save()"
powershell "$s=(New-Object -COM WScript.Shell).CreateShortcut('%FOLDER%\glory-bms-vehicle.lnk');$s.TargetPath='%~dp0%/bin/glory-bms-vehicle';$s.Save()"
powershell "$s=(New-Object -COM WScript.Shell).CreateShortcut('%FOLDER%\glory-can-charger.lnk');$s.TargetPath='%~dp0%/bin/glory-can-charger';$s.Save()"
powershell "$s=(New-Object -COM WScript.Shell).CreateShortcut('%FOLDER%\glory-can-vehicle.lnk');$s.TargetPath='%~dp0%/bin/glory-can-vehicle';$s.Save()"
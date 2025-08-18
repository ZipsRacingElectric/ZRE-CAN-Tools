@echo off
setlocal

:: Set the CANTOOLS directory environment variable
setx ZRE_CANTOOLS_DIR %~dp0%

:: Start menu directory for shortcuts
set "FOLDER=%APPDATA%\Microsoft\Windows\Start Menu\Programs\ZRE"
:: Delete the directory if it exists (removes old shortcuts).
if exist "%FOLDER%" rd /s /q "%FOLDER%"
:: Remake the directory
mkdir "%FOLDER%"

:: Create batch script shortcuts
call :createMenuShortcut cross-bms
call :createMenuShortcut cross-can
call :createMenuShortcut glory-bms-eeprom-charger
call :createMenuShortcut glory-bms-eeprom-vehicle
call :createMenuShortcut glory-bms-view-charger
call :createMenuShortcut glory-bms-view-vehicle
call :createMenuShortcut glory-can-charger
call :createMenuShortcut glory-can-vehicle
call :createMenuShortcut glory-vcu-vehicle
goto :eof

:: Function for creating a shortcut
:createMenuShortcut
powershell "$s=(New-Object -COM WScript.Shell).CreateShortcut('%FOLDER%\%~1.lnk');$s.TargetPath='%~dp0%/bin/%%~1';$s.Save()"
goto :eof
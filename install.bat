echo off
setlocal

:: Check OpenSSH is installed
where /q ssh
IF ERRORLEVEL 1 (
	echo OpenSSH is not installed. Please follow the steps provided in "doc/installing_openssh.pdf" to install, then re-run this installer.
	echo Press enter to close...
	pause >nul
	exit /B
)

:: Set the ZRE-CAN-Tools directory environment variable
setx ZRE_CANTOOLS_DIR "%~dp0%

:: Create and set the logging directory environment variable
setx ZRE_CANTOOLS_LOGGING_DIR "%userprofile%\Documents\ZRE"
mkdir "%userprofile%\Documents\ZRE"

:: Start menu directory for shortcuts
set "FOLDER=%APPDATA%\Microsoft\Windows\Start Menu\Programs\ZRE"
:: Delete the directory if it exists (removes old shortcuts).
if exist "%FOLDER%" rd /s /q "%FOLDER%"
:: Remake the directory
mkdir "%FOLDER%"

:: Create batch script shortcuts

call :createMenuShortcut dart-cli

call :createMenuShortcut zre24-can
call :createMenuShortcut zre24-bms

call :createMenuShortcut zr25-dashboard-vehicle
call :createMenuShortcut zr25-dashboard-charger
call :createMenuShortcut zr25-eeprom-vehicle
call :createMenuShortcut zr25-eeprom-charger
goto :complete

:: Function for creating a shortcut
:createMenuShortcut
echo Creating Shortcut %1...
powershell "$s=(New-Object -COM WScript.Shell).CreateShortcut('%FOLDER%/%~1.lnk');$s.TargetPath='conhost';$s.Arguments='%~dp0bin\%~1.bat';$s.Save()"
goto :eof

:complete
echo Installation Completed.
echo Press enter to close...
pause >nul
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

:: TODO(DiBacco): Verify Windows Implementation of Virtual Environment Config for Python Dependencies
:: Install python requirements
where /q py
IF ERRORLEVEL 1 (
	echo Python 3 is not installed or the Python launcher is unavailable. Install Python 3 and the py launcher, then re-run this installer.
	echo Press enter to close...
	pause >nul
	exit /B
)

py -3 -m venv "%~dp0venv"
IF ERRORLEVEL 1 (
	echo Failed to create the Python virtual environment. Check that Python 3 and the venv module are available, then re-run this installer.
	echo Press enter to close...
	pause >nul
	exit /B
)

"%~dp0venv\Scripts\python.exe" -m pip install -r "%~dp0requirements.txt"
IF ERRORLEVEL 1 (
	echo Failed to install Python requirements. Check that Python 3 and pip are available, then re-run this installer.
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

call :createMenuShortcut dart-cli.exe

call :createMenuShortcut zre24-can.bat
call :createMenuShortcut zre24-bms.bat

call :createMenuShortcut zr26-dashboard-vehicle.bat
call :createMenuShortcut zr26-dashboard-charger.bat
call :createMenuShortcut zr26-eeprom-vehicle.bat
call :createMenuShortcut zr26-eeprom-charger.bat
goto :complete

:: Function for creating a shortcut
:createMenuShortcut
echo Creating Shortcut %1...
powershell "$s=(New-Object -COM WScript.Shell).CreateShortcut('%FOLDER%/%~1.lnk');$s.TargetPath='cmd.exe';$s.Arguments='/k %~dp0bin\%~1';$s.Save()"
goto :eof

:complete
echo Installation Completed.
echo Press enter to close...
pause >nul
@echo off
setlocal enabledelayedexpansion

:: Default to 500kBaud
set "BAUD=%~1"
if [!BAUD!] == [] (
	set "BAUD=500000"
)

:: Initialize the CAN device
for /f %%i in ('%ZRE_CANTOOLS_DIR%/bin/init-can !BAUD! %ZRE_CANTOOLS_DEV%') do set "DEVICE=%%i"
if [!DEVICE!]==[] (
	echo Press enter to close...
	pause >nul
	exit /b -1
)

:: Set minimum terminal width
mode 160, 53

:: Start the application
%ZRE_CANTOOLS_DIR%/bin/can-eeprom-cli.exe !DEVICE! %ZRE_CANTOOLS_DIR%/config/zr25_glory/bms_config.json

echo Press enter to close...
pause >nul
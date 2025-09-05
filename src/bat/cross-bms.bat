@echo off
setlocal enabledelayedexpansion

:: Default to 1MBaud
set "BAUD=%~1"
if [!BAUD!] == [] (
	set "BAUD=1000000"
)

:: Initialize the CAN device
for /f %%i in ('%ZRE_CANTOOLS_DIR%/bin/init-can !BAUD! %ZRE_CANTOOLS_DEV%') do set "DEVICE=%%i"
if [!DEVICE!]==[] (
	echo Press enter to close...
	pause >nul
	exit /b -1
)

:: Set minimum terminal width
mode 133, 54

:: Start the application
%ZRE_CANTOOLS_DIR%/bin/bms-tui.exe !DEVICE! %ZRE_CANTOOLS_DIR%/config/zre24_cross/can.dbc %ZRE_CANTOOLS_DIR%/config/zre24_cross/bms_config.json

echo Press enter to close...
pause >nul
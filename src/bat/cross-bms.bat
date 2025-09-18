@echo off
setlocal enabledelayedexpansion

:: Arguments:
:: - 1 - Device name (optional)

:: Initialize the CAN device
for /f %%i in ('"%ZRE_CANTOOLS_DIR%/bin/can-init" 1000000 %~1') do set "DEVICE=%%i"
if [!DEVICE!]==[] (
	echo Press enter to close...
	pause >nul
	exit /b -1
)

:: Set minimum terminal width
mode 133, 54

:: Start the application
"%ZRE_CANTOOLS_DIR%/bin/bms-tui.exe" !DEVICE! "%ZRE_CANTOOLS_DIR%/config/zre24_cross/can.dbc" "%ZRE_CANTOOLS_DIR%/config/zre24_cross/bms_config.json"

echo Press enter to close...
pause >nul
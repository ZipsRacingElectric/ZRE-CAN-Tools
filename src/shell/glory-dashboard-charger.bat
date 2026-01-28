@echo off
setlocal enabledelayedexpansion

:: Arguments:
:: - 1 - Device name (optional)

:: Initialize the CAN device
for /f %%i in ('"%ZRE_CANTOOLS_DIR%/bin/can-init" 500000 %~1') do set "DEVICE=%%i"
if [!DEVICE!]==[] (
	echo Press enter to close...
	pause >nul
	exit /b -1
)

:: Start the application
"%ZRE_CANTOOLS_DIR%/bin/dashboard-gui" "%ZRE_CANTOOLS_DIR%/config/zr25_glory/dashboard_charger_config.json" !DEVICE! "%ZRE_CANTOOLS_DIR%/config/zr25_glory/can_charger.dbc"

echo Press enter to close...
pause >nul
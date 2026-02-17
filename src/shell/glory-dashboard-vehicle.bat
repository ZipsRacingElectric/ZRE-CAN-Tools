@echo off
setlocal enabledelayedexpansion

:: Arguments:
:: - 1 - Device name (optional)

IF [%1] == [] (
	"%ZRE_CANTOOLS_DIR%/bin/dashboard-gui.exe" "%ZRE_CANTOOLS_DIR%/config/zr25_glory/dashboard_vehicle_config.json" "COM*@1000000" "%ZRE_CANTOOLS_DIR%/config/zr25_glory/can_vehicle.dbc"
) ELSE (
	"%ZRE_CANTOOLS_DIR%/bin/dashboard-gui.exe" "%ZRE_CANTOOLS_DIR%/config/zr25_glory/dashboard_vehicle_config.json" "%1@1000000" "%ZRE_CANTOOLS_DIR%/config/zr25_glory/can_vehicle.dbc"
)

echo Press enter to close...
pause >nul
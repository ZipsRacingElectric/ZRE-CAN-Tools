@echo off
setlocal enabledelayedexpansion

:: Arguments:
:: - 1 - Device name (optional)

IF [%1] == [] (
	"%ZRE_CANTOOLS_DIR%/bin/dashboard-gui.exe" "%ZRE_CANTOOLS_DIR%/config/zr26/charger/dashboard_gui.json" "COM*@500000" "%ZRE_CANTOOLS_DIR%/config/zr26/charger/main.dbc"
) ELSE (
	"%ZRE_CANTOOLS_DIR%/bin/dashboard-gui.exe" "%ZRE_CANTOOLS_DIR%/config/zr26/charger/dashboard_gui.json" "%1@500000" "%ZRE_CANTOOLS_DIR%/config/zr26/charger/main.dbc"
)
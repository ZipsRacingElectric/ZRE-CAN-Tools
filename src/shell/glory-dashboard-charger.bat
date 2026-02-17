@echo off
setlocal enabledelayedexpansion

:: Arguments:
:: - 1 - Device name (optional)

IF [%1] == [] (
	"%ZRE_CANTOOLS_DIR%/bin/dashboard-gui.exe" "%ZRE_CANTOOLS_DIR%/config/zr25_glory/dashboard_charger_config.json" "COM*@500000" "%ZRE_CANTOOLS_DIR%/config/zr25_glory/can_charger.dbc"
) ELSE (
	"%ZRE_CANTOOLS_DIR%/bin/dashboard-gui.exe" "%ZRE_CANTOOLS_DIR%/config/zr25_glory/dashboard_charger_config.json" "%1@500000" "%ZRE_CANTOOLS_DIR%/config/zr25_glory/can_charger.dbc"
)

echo Press enter to close...
pause >nul
@echo off
setlocal enabledelayedexpansion

:: Arguments:
:: - 1 - Device name (optional)

:: Set minimum terminal width
mode 133, 54

IF [%1] == [] (
	"%ZRE_CANTOOLS_DIR%/bin/bms-tui.exe" "COM*@1000000" "%ZRE_CANTOOLS_DIR%/config/zre24/main.dbc" "%ZRE_CANTOOLS_DIR%/config/zre24/bms_config.json"
) ELSE (
	"%ZRE_CANTOOLS_DIR%/bin/bms-tui.exe" "%1@1000000" "%ZRE_CANTOOLS_DIR%/config/zre24/main.dbc" "%ZRE_CANTOOLS_DIR%/config/zre24/bms_config.json"
)
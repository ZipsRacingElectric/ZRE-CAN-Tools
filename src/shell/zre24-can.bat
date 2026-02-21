@echo off
setlocal enabledelayedexpansion

:: Arguments:
:: - 1 - Device name (optional)

:: Set minimum terminal width
mode 160, 53

IF [%1] == [] (
	"%ZRE_CANTOOLS_DIR%/bin/can-dbc-tui.exe" "COM*@1000000" "%ZRE_CANTOOLS_DIR%/config/zre24/main.dbc"
) ELSE (
	"%ZRE_CANTOOLS_DIR%/bin/can-dbc-tui.exe" "%1@1000000" "%ZRE_CANTOOLS_DIR%/config/zre24/main.dbc"
)
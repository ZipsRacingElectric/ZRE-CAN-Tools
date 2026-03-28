@echo off
setlocal enabledelayedexpansion

:: Set minimum terminal width
mode 133, 54

"%ZRE_CANTOOLS_DIR%/bin/bms-tui.exe" "%*" "COM*@1000000" "%ZRE_CANTOOLS_DIR%/config/zre24/main.dbc" "%ZRE_CANTOOLS_DIR%/config/zre24/bms_config.json"

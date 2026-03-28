@echo off
setlocal enabledelayedexpansion

:: Set minimum terminal width
mode 160, 53

"%ZRE_CANTOOLS_DIR%/bin/can-dbc-tui.exe" %* "COM*@1000000" "%ZRE_CANTOOLS_DIR%/config/zre24/main.dbc"
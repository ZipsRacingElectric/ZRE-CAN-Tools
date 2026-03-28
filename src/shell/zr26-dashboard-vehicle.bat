@echo off
setlocal enabledelayedexpansion

"%ZRE_CANTOOLS_DIR%/bin/dashboard-gui.exe" "%*" "%ZRE_CANTOOLS_DIR%/config/zr26/vehicle/dashboard_gui.json" "COM*@1000000" "COM*@1000000"
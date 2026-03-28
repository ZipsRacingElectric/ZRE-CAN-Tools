@echo off
setlocal enabledelayedexpansion

"%ZRE_CANTOOLS_DIR%/bin/dashboard-gui.exe" %* "%ZRE_CANTOOLS_DIR%/config/zr26/charger/dashboard_gui.json" "COM*@500000"
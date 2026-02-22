@echo off
setlocal enabledelayedexpansion

:: Arguments:
:: - 1 - Device name (optional)

:: Set minimum terminal width
mode 190, 53

IF [%1] == [] (
	"%ZRE_CANTOOLS_DIR%/bin/can-eeprom-cli.exe" "COM*@500000" "%ZRE_CANTOOLS_DIR%/config/zr25/bms_config.json"
) ELSE (
	"%ZRE_CANTOOLS_DIR%/bin/can-eeprom-cli.exe" "%1@500000" "%ZRE_CANTOOLS_DIR%/config/zr25/bms_config.json"
)

echo Press enter to close...
pause >nul
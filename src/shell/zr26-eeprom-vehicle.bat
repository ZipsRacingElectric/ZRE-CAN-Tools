@echo off
setlocal enabledelayedexpansion

:: Arguments:
:: - 1 - Device name (optional)

:: Set minimum terminal width
mode 190, 53

IF [%1] == [] (
	"%ZRE_CANTOOLS_DIR%/bin/can-eeprom-cli.exe" "COM*@1000000" "%ZRE_CANTOOLS_DIR%/config/zr26/vcu_config.json" "%ZRE_CANTOOLS_DIR%/config/zr26/bms_config.json" "%ZRE_CANTOOLS_DIR%/config/zr26/drs_config.json"
) ELSE (
	"%ZRE_CANTOOLS_DIR%/bin/can-eeprom-cli.exe" "%1@1000000" "%ZRE_CANTOOLS_DIR%/config/zr26/vcu_config.json" "%ZRE_CANTOOLS_DIR%/config/zr26/bms_config.json" "%ZRE_CANTOOLS_DIR%/config/zr26/drs_config.json"
)
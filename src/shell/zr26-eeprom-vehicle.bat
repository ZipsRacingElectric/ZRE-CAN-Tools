@echo off
setlocal enabledelayedexpansion

:: Set minimum terminal width
mode 190, 53

"%ZRE_CANTOOLS_DIR%/bin/can-eeprom-cli.exe" %* "COM*@1000000" "%ZRE_CANTOOLS_DIR%/config/zr26/vcu_config.json" "%ZRE_CANTOOLS_DIR%/config/zr26/bms_config.json" "%ZRE_CANTOOLS_DIR%/config/zr26/drs_config.json"

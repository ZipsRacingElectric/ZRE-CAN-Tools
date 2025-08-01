@echo off
setlocal enabledelayedexpansion

if "%~1" == "" (
    for /F "tokens=* usebackq" %%F IN (`call %ZRE_CANTOOLS_DIR%/bin/init-can 1000000`) do (
        set ZRE_CANTOOLS_DEV=%%F
    )
) else (
    for /F "tokens=* usebackq" %%F IN (`call %ZRE_CANTOOLS_DIR%/bin/init-can "%~1"`) do (
        set ZRE_CANTOOLS_DEV=%%F
    )
)

: Set minimum terminal width
mode 196, 53

echo Using CAN device: !ZRE_CANTOOLS_DEV!
%ZRE_CANTOOLS_DIR%/bin/bms-tui.exe !ZRE_CANTOOLS_DEV! %ZRE_CANTOOLS_DIR%/config/zre24_cross/can.dbc %ZRE_CANTOOLS_DIR%/config/zre24_cross/bms_config.json

echo Press enter to close...
pause >nul
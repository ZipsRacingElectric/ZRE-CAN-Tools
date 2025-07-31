@echo off
setlocal enabledelayedexpansion

if "%~1" == "" (
    for /F "tokens=* usebackq" %%F IN (`call %ZRE_CANTOOLS_DIR%/bin/init-can 500000`) do (
        set ZRE_CANTOOLS_DEV=%%F
    )
) else (
    for /F "tokens=* usebackq" %%F IN (`call %ZRE_CANTOOLS_DIR%/bin/init-can "%~1"`) do (
        set ZRE_CANTOOLS_DEV=%%F
    )
)

: Set minimum terminal width
mode 160, 53

echo Using CAN device: !ZRE_CANTOOLS_DEV!
%ZRE_CANTOOLS_DIR%/bin/can-dbc-tui.exe !ZRE_CANTOOLS_DEV! %ZRE_CANTOOLS_DIR%/config/zr25_glory/can_charger.dbc

echo Press enter to close...
pause >nul
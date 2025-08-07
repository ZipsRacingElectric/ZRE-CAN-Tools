@echo on
setlocal enabledelayedexpansion

:: Default to 1MBaud
set "BAUD=%~1"
if [!BAUD!] == [] (
	set "BAUD=1000000"
)

:: Initialize the CAN device
for /f %%i in ('init-can !BAUD! %ZRE_CANTOOLS_DEV%') do set "DEVICE=%%i"

:: Set minimum terminal width
mode 160, 53

:: Start the application
%ZRE_CANTOOLS_DIR%/bin/can-dbc-tui.exe !DEVICE! %ZRE_CANTOOLS_DIR%/config/zr25_glory/can_vehicle.dbc

echo Press enter to close...
pause >nul
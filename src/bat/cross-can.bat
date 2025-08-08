@echo off
setlocal enabledelayedexpansion

:: Force execution in cmd.exe instead of WindowsTerminal, the latter does not respect terminal size commands and breaks everything.
set id=%random%
title %id%
tasklist /v /fo csv | findstr "%id%" | findstr "cmd.exe"
if %errorlevel% == 1 start conhost "%~f0" & GOTO :EOF

:: Default to 1MBaud
set "BAUD=%~1"
if [!BAUD!] == [] (
	set "BAUD=1000000"
)

:: Initialize the CAN device
for /f %%i in ('%ZRE_CANTOOLS_DIR%/bin/init-can !BAUD! %ZRE_CANTOOLS_DEV%') do set "DEVICE=%%i"
if [!DEVICE!]==[] (
	echo Press enter to close...
	pause >nul
	exit /b -1
)

:: Set minimum terminal width
mode 160, 53

:: Start the application
%ZRE_CANTOOLS_DIR%/bin/can-dbc-tui.exe !DEVICE! %ZRE_CANTOOLS_DIR%/config/zre24_cross/can.dbc

echo Press enter to close...
pause >nul
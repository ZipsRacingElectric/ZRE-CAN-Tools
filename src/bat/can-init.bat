@echo off
setlocal enabledelayedexpansion

:: Check args
if "%~1"=="" (
	echo "Invalid arguments. Usage:" 1>&2
	echo "  can-init <Baud> <Device>" 1>&2
	echo "  can-init <Baud>" 1>&2
	exit /b -1
)

if "%~2"=="" (
	for /f "usebackq tokens=1*" %%i in (`PowerShell.exe "Get-WmiObject Win32_SerialPort | Select-Object deviceid"`) do (
		set "DEVICE=%%i@%~1"
		"%ZRE_CANTOOLS_DIR%/bin/can-dev-cli" -q !DEVICE! 2> nul && echo !DEVICE! && exit /b 0
	)

	:: No device found, exit
	echo "No CAN device was detected." 1>&2
	exit /b -1
) else (
	:: Postfix baudrate to device name
	echo %~2@%~1
	exit /b 0
)

echo "Unrecognized CAN device '%~2'" 1>&2
exit /b -1
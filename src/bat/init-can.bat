@echo off
setlocal enabledelayedexpansion

:: Check args
if "%~1"=="" (
	echo "Invalid arguments. Usage:" 1>&2
	echo "  init-can <Baud> <Device>" 1>&2
	exit /b -1
)

if "%~2"=="" (
	for /f "tokens=1* delims==" %%I in ('wmic path win32_pnpentity get caption /format:list ^| find "COM"') do (
			call :setCOM "%%~J"
		%ZRE_CANTOOLS_DIR%/bin/can-dev-cli -q !DEVICE!@%~1 2> nul && echo !DEVICE!@%~1 && exit /b 0
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

:setCOM <WMIC_output_line>
:: sets _COM#=line
setlocal
set "str=%~1"
set "num=%str:*(COM=%"
set "num=%num:)=%"
set str=%str:(COM=&rem.%
endlocal & set "DEVICE=COM%num%"
goto :EOF
@echo off
setlocal enabledelayedexpansion

:: Check args
if "%~1"=="" ( goto printUsage )
if "%~1"=="-h" ( goto printUsage )
if "%~1"=="--help" ( goto printUsage )
if "%~1"=="help" ( goto printUsage )

:: Start the application (no filter IDs)
if "%~2"=="" (
	%ZRE_CANTOOLS_DIR%\bin\can-dev-cli -d %~1
	exit /b 0
)

:: Start the application (with filter IDs)
%ZRE_CANTOOLS_DIR%\bin\can-dev-cli -d=%~2 %~1
exit /b 0

:printUsage
echo "Usage:" 1>&2
echo "  can-dump <Device Name>" 1>&2
echo "  can-dump <Device Name> <CAN IDs>" 1>&2
exit /b -1
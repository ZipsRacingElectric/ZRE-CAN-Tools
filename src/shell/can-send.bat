@echo off
setlocal enabledelayedexpansion

:: TODO(Barach): This is broken because batch is batch...
:: TODO(Barach): Consider distributing mintty...

:: Check args
if "%~1"=="" ( goto printUsage )
if "%~2"=="" ( goto printUsage )

:: Start the application
%ZRE_CANTOOLS_DIR%\bin\can-dev-cli -t=%~2 %~1
exit /b 0

:printUsage
echo "Usage:" 1>&2
echo "  can-send <Device Name> <CAN ID>[<Byte 0>,<Byte 1>,...<Byte N>]" 1>&2
echo "  can-init <Device Name> <CAN ID>[<Byte 0>,<Byte 1>,...<Byte N>]@<Count>,<Frequency>" 1>&2
exit /b -1
@echo off
setlocal enabledelayedexpansion

if not "%~1"=="" (
    for /f "tokens=1 delims=," %%a in ("%ZRE_CANTOOLS_DEV%") do (
        set ZRE_CANTOOLS_DEV=%%a
    )
    set ZRE_CANTOOLS_DEV=!ZRE_CANTOOLS_DEV!,%~1%
) else (
    for /f "tokens=1 delims=," %%a in ("%ZRE_CANTOOLS_DEV%") do (
        set ZRE_CANTOOLS_DEV=%%a
    )
    set ZRE_CANTOOLS_DEV=!ZRE_CANTOOLS_DEV!,1000000
)

echo !ZRE_CANTOOLS_DEV!
setlocal

:: Delete the start menu directory if it exists.
set "FOLDER=%APPDATA%\Microsoft\Windows\Start Menu\Programs\ZRE"
if exist "%FOLDER%" rd /s /q "%FOLDER%"
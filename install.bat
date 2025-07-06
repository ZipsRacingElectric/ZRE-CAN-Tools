setx ZRE_CANTOOLS_DIR %~dp0%
setx ZRE_CANTOOLS_DEV /dev/ttyS2
powershell "$s=(New-Object -COM WScript.Shell).CreateShortcut('%userprofile%\Desktop\cross-bms.lnk');$s.TargetPath='%~dp0%/bin/cross-bms';$s.Save()"
powershell "$s=(New-Object -COM WScript.Shell).CreateShortcut('%userprofile%\Desktop\cross-can.lnk');$s.TargetPath='%~dp0%/bin/cross-can';$s.Save()"
powershell "$s=(New-Object -COM WScript.Shell).CreateShortcut('%userprofile%\Desktop\glory-bms-charger.lnk');$s.TargetPath='%~dp0%/bin/glory-bms-charger';$s.Save()"
powershell "$s=(New-Object -COM WScript.Shell).CreateShortcut('%userprofile%\Desktop\glory-bms-vehicle.lnk');$s.TargetPath='%~dp0%/bin/glory-bms-vehicle';$s.Save()"
powershell "$s=(New-Object -COM WScript.Shell).CreateShortcut('%userprofile%\Desktop\glory-can-charger.lnk');$s.TargetPath='%~dp0%/bin/glory-can-charger';$s.Save()"
powershell "$s=(New-Object -COM WScript.Shell).CreateShortcut('%userprofile%\Desktop\glory-can-vehicle.lnk');$s.TargetPath='%~dp0%/bin/glory-can-vehicle';$s.Save()"
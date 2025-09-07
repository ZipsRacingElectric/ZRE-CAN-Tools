ZRE-CAN-Tools is the application layer of Zips Racing's electrical systems.
This project is a combination of libraries and applications that aim to
simplify the interaction with firmware written by Zips Racing.

- Installation ----------------------------------------------------------------

To install this, run the "install" file in this directory. If you are updating
from a previous version, all you need to do is delete the old directory then
install this one normally (no need to uninstall).

On Linux, the installer will need run with root permissions (use sudo). After
the installer has completed, the system will need rebooted.

- Uninstallation --------------------------------------------------------------

To uninstall this, run the "uninstall" file in this directory. After it has
run, you may delete this directory.

On Linux, the uninstaller will need run with root permissions.

- Usage -----------------------------------------------------------------------

On Windows, after running the installed, a new folder will appear in your start
menu (under "All") called "ZRE". This folder contains all the applications that
were installed.

On Linux, the installed applications are added to your system path. This means
they can be run from a terminal.

- ZR25 "Gloria" ---------------------------------------------------------------

All the applications for ZR25 start with the prefix "glory" (for Gloria). The
postfix of each application indicates whether it is intended for use with the
vehicle itself (ends with "vehicle"), or the charging cart (ends with
"charger").

 - glory-bms-view-vehicle   - Application for monitoring the BMS (in vehicle).
 - glory-bms-view-charger   - Application for monitoring the BMS (on charging
                              cart).
 - glory-can-vehicle        - Application for monitoring the vehicle CAN bus.
 - glory-can-charger        - Application for monitoring the charger CAN bus.
 - glory-bms-eeprom-vehicle - Application for configuring the BMS (in vehicle).
 - glory-bms-eeprom-charger - Application for configuring the BMS (on charger).
 - glory-vcu-vehicle        - Application for configuring the VCU (in vehicle).
 - glory-drs-vehicle        - Application for configuring the DRS (in vehicle).

- ZRE24 "Christine" -----------------------------------------------------------

All the applications for ZR24 start with the prefix "cross" (for Christine).

 - cross-bms - Application for monitoring the BMS.
 - cross-can - Application for monitoring the CAN bus.
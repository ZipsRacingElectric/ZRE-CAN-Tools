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

- DART ------------------------------------------------------------------------

Zips Racing's modern vehicles are equipped with a custom dashboard and data
logging system (referred to as the "DART"). The dart-cli application is used to
interact with this system. See the doc/dart_user_manual.pdf file for
information about how to use the DART.

- dart-cli                      - Application for interacting with the DART of
                                  any vehicle.

- ZR25 "Gloria" ---------------------------------------------------------------

All the applications for ZR25 start with the prefix "zr25". The postfix of each
application indicates whether it is intended for use with the vehicle itself
(ends with "vehicle"), or the charging cart (ends with "charger").

 - zr25-dashboard-vehicle       - Dashboard for the vehicle.
 - zr25-dashboard-charger       - Dashboard for the charger.
 - zr25-eeprom-vehicle          - Application for configuring the EEPROM of
                                  devices in the vehicle.
 - zr25-eeprom-charger          - Application for configuring the EEPROM of the
                                  BMS on the charging cart.

- ZRE24 "Christine" -----------------------------------------------------------

All the applications for ZR24 start with the prefix "zre24".

 - zre24-bms                    - Application for monitoring the BMS.
 - zre24-can                    - Application for monitoring the CAN bus.
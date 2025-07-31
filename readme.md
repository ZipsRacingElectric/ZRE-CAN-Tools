# CAN Tools - Zips Racing 2025
## Usage
### ZR25 Shell Scripts
A set of shell scripts are provided to simplify usage of the applications.
 - ```glory-bms-vehicle``` - Opens the BMS TUI for the BMS of ZR25, configured for the vehicle's CAN bus.
 - ```glory-bms-charger``` - Opens the BMS TUI for the BMS of ZR25, configured for the charger's CAN bus.
 - ```glory-can-vehicle``` - Opens the CAN DBC TUI configured for the vehicle CAN bus of ZR25.
 - ```glory-can-charger``` - Opens the CAN DBC TUI configured for the charger CAN bus of ZR25.

### ZRE24 Shell Scripts
 - ```cross-bms``` - Opens the BMS TUI for the BMS of ZRE24.
 - ```cross-can``` - Opens the CAN DBC TUI configured for the CAN bus of ZRE24.

### CAN DBC CLI
```can-dbc-cli <device name> <DBC file path>```
This program is used to interact with a CAN node in real-time. Received messages are parsed and stored in a relational database which can be queried. Arbitrary messages can be transmitted by the user.

### CAN DBC TUI
```can-dbc-tui <device name> <DBC file path>```
This program is used to monitor the activity of a CAN bus in realtime.

### CAN EEPROM CLI
```
Usage:

can-eeprom-cli <options> <device name> <config JSON path>

Options:

    -p=<data JSON path>   - Programming mode. Reads a data JSON from the
                            specified path and programs the key-value pairs to
                            the device. If no path is specified, the file is
                            read from stdin.
    -r=<data JSON path>   - Recovery mode. Writes the EEPROM's memory to a data
                            JSON file. If no path is specified, the file is
                            written to stdout.
    -v                    - Verbose. Enables more verbose output to stderr for
                            debugging.
    -h                    - Help. Prints this text.
```

This program is used to program a device's EEPROM via CAN bus.

### Command-line Arguments
```
<device name>         - The adapter-specific identity of the CAN device.
    can*              - SocketCAN device, must be already initialized and
                        setup. Ex. 'can0'.
    vcan*             - Virtual SocketCAN device, must be already initialized
                        and setup.
    COM*,<baud>       - SLCAN device, must be a CANable device. CAN baudrate is
                        indicated by the baud field. Ex 'COM3,1000000'.
<DBC file path>       - The path to the DBC file to use.
<config JSON path>    - The configuration JSON file to use. Configuration files
                        indicate the identity and unit-specific parameters of
                        a CAN node.
<data JSON path>      - The data JSON file to use. Data files contain the
                        values of the unit-specific parameters of a CAN node.
```

## Installation (For General Usage)
### Windows
- Go the the [Releases](https://github.com/ZipsRacingElectric/CAN-Tools-2025/releases) section on github.
- Download the latest version for Windows.
- Extract the zip file to a permanent location, ex. ```C:/zre_cantools```.
- Run the ```install.bat``` script.
- When prompted, select a convenient directory to place the shortcuts, recommended Desktop or Downloads folder.

### Linux
- Go the the [Releases](https://github.com/ZipsRacingElectric/CAN-Tools-2025/releases) section on Github.
- Download the latest version for Linux.
- Extract the zip file to a permanent location, ex. ```~/zre-cantools```.
- Define the ```ZRE_CANTOOLS_DIR``` environment variable to point to the location of this directory.
- Define the ```ZRE_CANTOOLS_DEV``` environment variable to the default value of ```<device name>``` (see 'Command-line Arguments' for details).

## Installation (For Development)
- Clone this repo using Github's SSH URL ```git clone <SSH URL>```
- Define the ```ZRE_CANTOOLS_DIR``` environment variable to point to the location of this directory.
- Define the ```ZRE_CANTOOLS_DEV``` environment variable to the default value of ```<device name>``` (see 'Command-line Arguments' for details).
- Perform the OS-specific steps setup below before continuing with these steps.
- Run ```make``` to compile all of the programs.
- Add the newly created ```bin``` directory to your system path.

### For Linux
Install all of the following dependencies, if not already installed:
- libnursesw-dev (NCurses development library with wide character support)

### For Windows
Some dependencies of this project are not natively built for Windows. A solution to this is to use MSYS2, a collection of tools and libraries that provide a POSIX-like development environment for Windows.
- Download and run the MSYS2 installer from [Github](https://github.com/msys2/msys2-installer/releases/).
- When finished, a terminal should open, if not, open one by searching 'MSYS2 UCRT64' from the start menu.
- In said terminal, run ```pacman -S --needed base-devel mingw-w64-ucrt-x86_64-toolchain```. When prompted, select default (all) packages.
- Add the ```msys64\ucrt64\bin``` and ```msys64\usr\bin``` directories to your system path.
- From this point, all commands can be run from a standard command prompt.

## Compilation
Use ```make``` to compile all of the programs.

Use ```make bin\<executable file>``` to compile a specific program.

Use ```make -B``` to re-compile all of the programs. Note that this may be required after making library modifications.

## Directory Structure
```
.
├── bin                        - Output directory for compilation. Final
│                                applications will be placed in here.
├── config                     - Application-specific configuration files.
│   ├── zre24_cross            - Configuration files for ZRE24, 'Christine'.
│   └── zr25_glory             - Configuration files for ZR25, 'Gloria'.
├── lib                        - Libraries that are dependencies of the
│   │                            different applications. Some custom, some
│   │                            external.
│   ├── bms                    - Library for interacting with BMS can nodes.
│   ├── can_database           - Library for defining a CAN database from a DBC
│   │                            file.
│   ├── can_device             - Library for abstracting different CAN
│   │                            adapters.
│   ├── can_eeprom             - Library for interacting with a device's EEPROM
│   │                            via CAN bus.
│   ├── cjson                  - Library for working with JSON files. Upstream:
│   │                            https://github.com/DaveGamble/cJSON.
│   ├── serial_can             - Library for interacting with SLCAN devices.
│   │                            Upstream: https://github.com/mac-can/SerialCAN
│   └── error_codes.h          - Error code definitions common to all
│                                libraries.
├── makefile                   - Project-level makefile for compiling all
│                                applications.
└── src                        - Application-specific source code.
    ├── can_dbc_cli            - Source code for the can-dbc-cli application.
    ├── can_dbc_tui            - Source code for the can-dbc-tui application.
    └── can_eeprom_cli         - Source code for the can-eeprom-cli
                                 application.
```
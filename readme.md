# CAN Tools - Zips Racing 2025
## Usage
### ZR25 Shell Scripts
A set of shell scripts are provided to simplify usage of the applications.
 - ```. glory-init-vehicle``` - Initializes a CAN device connected to the CAN bus of ZR25.
 - ```. glory-init-charger``` - Initializes a CAN device connected to the charger of ZR25.
 - ```glory-bms``` - Opens the BMS TUI configured for the BMS of ZR25.
 - ```glory-vehicle``` - Opens the CAN DBC TUI configured for the vehicle CAN bus of ZR25.
 - ```glory-charger``` - Opens the CAN DBC TUI configured for the charger CAN bus of ZR25.

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
    /dev/tty*,<baud>  - SLCAN device, must be a CANable device. CAN baudrate is
                        indicated by the baud field. Ex '/dev/ttyS2,1000000'.
<DBC file path>       - The path to the DBC file to use.
<config JSON path>    - The configuration JSON file to use. Configuration files
                        indicate the identity and unit-specific parameters of
                        a CAN node.
<data JSON path>      - The data JSON file to use. Data files contain the
                        values of the unit-specific parameters of a CAN node.
```

## Setup
- Clone this repo using github's SSH URL ```git clone <SSH URL>```
- Define the ```ZRE_CANTOOLS_DIR``` environment variable to point to the location of this directory.
- Define the ```ZRE_CANTOOLS_DEV``` environment variable to the default value of ```<device name>``` (see 'Command-line Arguments' for details).
- If you are using Windows, perform the 'For Windows' setup
- Run ```make``` to compile all of the programs.
- Add ```bin``` to your system path.

### For Windows
Some dependicies of this project are not natively built for Windows. A solution to this is to use Cygwin, a POSIX compatibility layer that allows these programs to be run in Windows.

#### Cygwin
- Download and install Cygwin from https://cygwin.com/.
- Install the following packages:
    ```Base```
    ```Devel/gcc-core```
    ```Devel/gcc-g++```
    ```Devel/Make```
    ```Devel/libncurses-devel```
- Add ```C:\cygwin64\bin\``` to your system path.
- From a command-line, run ```bash --version``` to validate Cygwin has been installed.

#### Make
- Make should be installed with Cygwin.
- From a command-line, run ```make -v``` to validate make has been installed.

### Compilation
Use ```make``` to compile all of the programs.
Use ```make bin\<executable file>``` to compile a specific program.
Use ```make -B``` to re-compile all of the programs. Note that this may be required after making library modifications.

## Directory Structure
```
.
├── bin                        - Output directory for compilation. Final
│                                applications will be placed in here.
├── config                     - Application-specific configuration files.
│    └── glory                 - Configuration files for ZR25, 'gloria'
├── lib                        - Libraries that are dependencies of the
│   │                            different applications. Some custom, some
│   │                            external.
│   ├── bms                    - Library for interacting with BMS can nodes.
│   ├── can_database           - Library for defining a CAN database from a DBC
│   │                            file
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
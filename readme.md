# CAN Tools - Zips Racing 2025
## Usage
### CAN DBC CLI
```can-dbc-cli <device name> <DBC file path>```
Where ```device name``` is the name of the network device to use (ex. ```can0```), and ```DBC file path``` is the path to the CAN DBC file to use.

This program is used to interact with a CAN node in real-time. Received messages are parsed and stored in a relational database which can be queried. Arbitrary messages can be transmitted by the user.

### CAN DBC TUI
```can-dbc-tui <device name> <DBC file path>```
Where ```device name``` is the name of the network device to use (ex. ```can0```), and ```DBC file path``` is the path to the CAN DBC file to use.

This program is used to monitor the activity of a CAN bus in realtime.

### CAN EEPROM CLI
```
Usage:

can-eeprom-cli <options> <device name> <config JSON path>
    Where: 'device name' is the name of the network device to use (ex. 'can0').
           'config JSON path' is the path of the device's configuration file.

Options:

    -i          - Interactive mode. Prompts the user for commands.
    -p=<path>   - Programming mode. Reads a data JSON from the specified path
                  and programs the key-value pairs to the device. If no path
                  is specified, the file is read from stdin.
    -v          - Verbose. Enables more verbose output to stderr for debugging.
    -h          - Help. Prints this text.
```

This program is used to program a device's EEPROM via CAN bus.

## Compilation

Use ```make``` to compile all of the programs.

Use ```make bin\<executable file>``` to compile a specific program.

Use ```make -B``` to re-compile all of the programs.

Use ```sudo ip link set up can0 type can bitrate 1000000``` to setup the ```can0``` device at 1 megabaud.

Use ```sudo ip link add dev vcan type vcan``` followed by ```sudo ip link up vcan``` to create and setup a virtual CAN device named ```vcan```.

## Directory Structure
```
.
├── bin                        - Output directory for compilation. Final applications will be placed in here.
├── config                     - Application-specific configuration files. Contains a DBC file for the vehicle's CAN bus and
│                                device configuration and device data JSONs for the EEPROM programmer.
├── lib                        - Libraries that are dependencies of the different applications. Some custom, some external.
│   ├── can                    - CAN related code is implemented in this library. Basic objects for interacting with a CAN bus.
│   ├── cjson                  - Library for working with JSON files. Upstream: https://github.com/DaveGamble/cJSON.
│   └── error_codes.h          - Error code definitions common to all libraries.
├── makefile                   - Project-level makefile for compiling all applications.
└── src                        - Application-specific source code.
    ├── can_dbc_cli            - Source code for the can-dbc-cli application.
    ├── can_dbc_tui            - Source code for the can-dbc-tui application.
    └── can_eeprom_cli         - Source code for the can-eeprom-cli application.
```
# ZRE-CAN-Tools - Zips Racing

ZRE-CAN-Tools is the application layer of Zips Racing's electrical systems. This project is a combination of libraries and applications that aim to simplify the interaction with firmware written by Zips Racing.

## Usage
### ZR25 Shell Scripts

A set of shell scripts are provided to simplify usage of the applications.
 - `glory-bms-view-vehicle` - Application for monitoring the BMS of ZR25, configured for the vehicle's CAN bus.
 - `glory-bms-view-charger` - Application for monitoring the BMS of ZR25, configured for the charger's CAN bus.
 - `glory-can-vehicle` - Application for monitoring the vehicle CAN bus of ZR25.
 - `glory-can-charger` - Application for monitoring the charger CAN bus of ZR25.
 - `glory-bms-eeprom-vehicle` - Application for configuring the BMS of ZR25, configured for the vehicle's CAN bus.
 - `glory-bms-eeprom-charger` - Application for configuring the BMS of ZR25, configured for the charger's CAN bus.
 - `glory-vcu-vehicle` - Application for configuring the VCU of ZR25, configured for the vehicles's CAN bus.
 - `glory-drs-vehicle` - Application for configuring the DRS of ZR25, configured for the vehicles's CAN bus.

### ZRE24 Shell Scripts

 - `cross-bms` - Application for monitoring the BMS of ZRE24.
 - `cross-can` - Application for monitoring the CAN bus of ZRE24.

### CAN-DBC-TUI

`can-dbc-tui <Device Name> <DBC file path>`
This program is used to monitor the activity of a CAN bus in real-time.

### CAN-DEV-CLI

```
Usage: can-dev-cli <Options> <Device Name>

Options:
    -t=<CAN Frame>
        Transmits a single CAN frame.

    -t=<CAN Frame>@<Count>,<Freq>
        Transmits <Count> CAN frames at the frequency of <Freq> Hertz.

    -r  Receives the first available CAN message.

    -r=@<Count>
        Receives the first <Count> available CAN messages.

    -r=<CAN ID 0>,<CAN ID 1>,...<CAN ID N>
        Receives the first available CAN message matching any of the given IDs.

    -r=<CAN ID 0>,<CAN ID 1>,...<CAN ID n>@<Count>
        Receives the first <Count> available CAN messages matching any of the
        given IDs.

    -d  Dumps all received CAN messages.

    -d=<CAN ID 0>,<CAN ID 1>,...<CAN ID N>
        Dumps all received CAN messages matching any of the given IDs.

Examples:

    Dump all received CAN messages:
        can-dev-cli -d <Device Name>

    Periodically transmit a CAN message (50 times at 10 Hz):
        can-dev-cli -t=0x123[0xAB,0xCD]@50,10 <Device Name>

    Dump all received CAN messages from a list:
        can-dev-cli -d=0x005,0x006,0x007,0x008 <Device Name>

    Transmit a remote transmission request frame:
        can-dev-cli -t=0x123r <Device Name>

    Receive a frame with an extended CAN ID:
        can-dev-cli -r=0xABCDEFx <Device Name>

```

### CAN-Dump

`can-dump <Device Name>` - Dumps all received CAN messages to the standard output.

`can-dump <Device Name> <CAN IDs>` - Dumps all received CAN messages matching a list of IDs to the standard output.

### CAN-Send

`can-send <Device Name> <CAN ID>[<Byte 0>,<Byte 1>,...<Byte N>]` - Transmits a single CAN message.

`can-send <Device Name> <CAN ID>[<Byte 0>,<Byte 1>,...<Byte N>]@<Count>,<Frequency>` - Transmits a specified number of CAN messages at a specified frequency, in Hertz.

### CAN-EEPROM-CLI

```
Usage:

can-eeprom-cli <options> <device name> <config JSON path>

Options:

    -p=<Data JSON path>   - Programming mode. Reads a data JSON from the
                            specified path and programs the key-value pairs to
                            the device. If no path is specified, the file is
                            read from stdin.
    -r=<Data JSON path>   - Recovery mode. Writes the EEPROM's memory to a data
                            JSON file. If no path is specified, the file is
                            written to stdout.
    -v                    - Verbose. Enables more verbose output to stderr for
                            debugging.
    -h                    - Help. Prints this text.
```

This program is used to program a device's EEPROM via CAN bus.

### CAN-DBC-CLI

`can-dbc-cli <Device Name> <DBC file path>`
This program is used to interact with a CAN node in real-time. Received messages are parsed and stored in a relational database which can be queried. Arbitrary messages can be transmitted by the user.

### BMS-TUI

`bms-tui <Device Name> <DBC file path> <Config JSON path>`
This program is used to monitor a battery management system in real-time.

### Command-line Parameters

```
<Device Name>         - The adapter-specific identity of the CAN device.
    can*              - SocketCAN device, must be already initialized and
                        setup. Ex. 'can0'.
    vcan*             - Virtual SocketCAN device, must be already initialized
                        and setup.
    <port>@<baud>     - SLCAN device, must be a CANable device. CAN baudrate is
                        indicated by the baud field. Ex 'COM3@1000000' for
                        Windows and '/dev/ttyACM0@1000000' for Linux.

<CAN Frame>           - A CAN frame. May be a data frame or RTR frame, based on
                        the ID. Takes the following format:
    <CAN ID>[<Byte 0>,<Byte 1>,...<Byte N>]

<Byte i>              - The i'th byte of a frame's data payload, indexed in
                        little-endian (aka Intel format). May be either decimal
                        or hexadecimal (prefixed with '0x').

<CAN ID>              - The identifier of a frame.
    <SID>             - Standard CAN ID, may be decimal or hexadecimal (prefixed
                        with '0x').
    <SID>r            - Standard CAN ID, for an RTR frame.
    <EID>x            - Extended CAN ID.
    <EID>xr           - Extended CAN identifier, for an RTR frame.

<DBC file path>       - The path to the DBC file to use.

<Config JSON path>    - The configuration JSON file to use. Configuration files
                        indicate the identity and unit-specific parameters of
                        a CAN node.

<Data JSON path>      - The data JSON file to use. Data files contain the
                        values of the unit-specific parameters of a CAN node.
```

## Installation (For General Usage)

- Go the the [Releases](https://github.com/ZipsRacingElectric/CAN-Tools-2025/releases) section on GitHub.
- Download the latest version for your OS.
- Extract the archive to a permanent location, ex. `%userprofile%/ZRE`. Note this cannot be inside a OneDrive folder.
- See the `readme.txt` file for installation & usage instructions.

## Installation (For Development)

- Clone this repo using GitHub's SSH URL `git clone <SSH URL>`
- Perform the OS-specific steps setup below before continuing with these steps.
- Run `make` to compile all of the programs.
- Run the `install` script to create the needed environment variables.
  - Note that on Linux, you will need to logout and log back in after this.
- On Windows, it is useful to add the `bin` directory to your system path (not needed, just conventient).

### For Linux

Install all of the following dependencies, if not already installed:
- `libncurses-dev` (Debian) or `ncurses` (Arch) - NCurses development library with wide character support.
- `libgtk-4-dev` (Debian) or `gtk4` (Arch) - GTK 4 development library.

### For Windows

Some dependencies of this project are not natively built for Windows. A solution to this is to use MSYS2, a collection of tools and libraries that provide a POSIX-like development environment for Windows.
- Download and run the MSYS2 installer from [GitHub](https://github.com/msys2/msys2-installer/releases/).
- When finished, a terminal should open, if not, open one by searching 'MSYS2 UCRT64' from the start menu.
- In said terminal, run `pacman -S --needed base-devel mingw-w64-ucrt-x86_64-toolchain mingw-w64-ucrt-x86_64-gtk4`. When prompted, select default (all) packages.
- Add the `msys64\ucrt64\bin` and `msys64\usr\bin` directories to your system path.
- Create the `MSYS_BIN` environment variable defined to the `msys64\ucrt64\bin` directory.
- From this point, all further commands can be run from command prompt.

## Compilation

Use `make` or `make bin` to compile all of the applications.

Use `make lib` to compile just the libraries.

Use `make -B` to re-compile all of the programs. Note that this may be required after making modifications to the build system.

Use `make release` to create a release of the project.

Use the `install` script to recreate the application shortcuts / setup environment variables.

## Project Structure

```
.
├── bin                        - Output directory for compilation. Final
│                                applications will be placed in here.
├── config                     - Application-specific configuration files.
│   │                            These define the configurations that specific
│   │                            vehicles or firmware use.
│   ├── zre24_cross            - Configuration files for ZRE24, 'Christine'.
│   └── zr25_glory             - Configuration files for ZR25, 'Gloria'.
├── lib                        - Libraries that are dependencies of the
│   │                            different applications. Some custom, some
│   │                            external.
│   ├── bms                    - Library for interacting with BMS firmware.
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
    ├── bat                    - Helper batch scripts for Windows usage.
    ├── bms_tui                - Source code for the bms-tui application.
    ├── can_dbc_cli            - Source code for the can-dbc-cli application.
    ├── can_dbc_tui            - Source code for the can-dbc-tui application.
    ├── can_dev_cli            - Source code for the can-dev-cli application.
    ├── can_eeprom_cli         - Source code for the can-eeprom-cli
    │                            application.
    └── sh                     - Helper shell scripts for Linux usage.
```

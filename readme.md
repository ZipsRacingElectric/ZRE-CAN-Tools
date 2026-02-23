# ZRE-CAN-Tools - Zips Racing

ZRE-CAN-Tools is the application layer of Zips Racing's electrical systems. This project is a combination of libraries and applications that aim to simplify the interaction with firmware written by Zips Racing.

## Usage

### Shell Scripts

A set of shell scripts are provided to simplify usage of the applications.

See [doc/readme_release.txt](doc/readme_release.txt) for documentation on the shell scripts.

### Applications

For all applications, use `-h` to display the help page. For example: `can-dev-cli -h`.

`dart-cli` - Command-line application for interacting with Zips Racing's "DART" data acquisition system. See [doc/dart_user_manual.pdf](doc/dart_user_manual.pdf) for more details.

`can-dev-cli` - Command-line application for controlling a CAN adapter. This program exposes 'raw' access to a CAN bus, that is, the user can directly transmit and receive CAN frames.

`can-dbc-tui` - Terminal user interface used to monitor the activity of a CAN bus in real-time.

`can-dbc-cli` - Command-line interface used to interact with a CAN bus. Received messages are parsed and stored in a relational database which can be queried. Arbitrary messages can be transmitted by the user.

`can-eeprom-cli` - Command-line interface used to program a device's EEPROM via CAN bus.

`can-bus-load` - Application for estimating the load of a CAN bus. CAN bus load is defined as the percentage of time the CAN bus is in use. This calculator estimates both the minimum and maximum bounds of this load.

`can-mdf-logger` - Application for logging the traffic of a CAN bus to an MDF file. This application also can transmit a status message containing the logging session and CAN bus's load / error count.

`bms-tui` - Terminal user interface for monitoring a battery management system in real-time.

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
- On Windows, it is useful to add the `bin` directory to your system path (not needed, just convenient).

### For Linux

Install all of the following dependencies, if not already installed:
- `libncurses-dev` (Debian) or `ncurses` (Arch) - NCurses development library with wide character support.
- `libgtk-4-dev` (Debian) or `gtk4` (Arch) - GTK 4 development library.
- `ssh` (Debian) or `openssh` (Arch) - OpenSSH Client.

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
│   ├── zre24                  - Configuration files for ZRE24, 'Christine'.
│   └── zr25                   - Configuration files for ZR25, 'Gloria'.
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
│   ├── can_node               - Library for general interactings with devices
│   │                            in a CAN bus.
│   ├── cjson                  - Library for working with JSON files. Upstream:
│   │                            https://github.com/DaveGamble/cJSON.
│   ├── mdf                    - Library for working with the MDF standard.
│   └── serial_can             - Library for interacting with SLCAN devices.
│                                Upstream: https://github.com/mac-can/SerialCAN
├── makefile                   - Project-level makefile for compiling all
│                                applications.
└── src                        - Application-specific source code.
```

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

### CAN EEPROM Programmer
```can-eeprom-programmer <device name>```

This program is used to program a device's EEPROM via CAN bus.
## Compilation
Use ```make``` to compile all of the programs.

Use ```make bin\<Executable File>``` to compile a specific program.

Use ```make -B``` to re-compile all of the programs.

Use ```sudo ip link set up can0 type can bitrate 1000000``` to setup the ```can0``` device at 1 megabaud.
#!/bin/bash

# Helper for printing to stderr
echoerr() { printf "%s\n" "$*" >&2; }

# Check args
if [ "$1" = "" ] || [ "$2" = "" ]; then
	echoerr "Usage:"
	echoerr "can-send <Device Name> <CAN ID>[<Byte 0>,<Byte 1>,...<Byte N>]"
	echoerr "can-send <Device Name> <CAN ID>[<Byte 0>,<Byte 1>,...<Byte N>]@<Count>,<Frequency>"
	exit -1
fi

# Start the application
$ZRE_CANTOOLS_DIR/bin/can-dev-cli -t=$2 $1
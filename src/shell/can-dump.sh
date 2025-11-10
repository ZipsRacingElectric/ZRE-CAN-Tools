#!/bin/bash

# Helper for printing to stderr
echoerr() { printf "%s\n" "$*" >&2; }

# Check args
if [ "$1" = "" ] || [ "$1" = "-h" ] || [ "$1" = "--help" ] || [ "$1" = "help" ]; then
	echoerr "Usage:"
	echoerr "  can-dump <Device Name>"
	echoerr "  can-dump <Device Name> <CAN IDs>"
	exit -1
fi

# Start the application
if [ "$2" = "" ]; then
	$ZRE_CANTOOLS_DIR/bin/can-dev-cli -d $1
else
	$ZRE_CANTOOLS_DIR/bin/can-dev-cli -d=$2 $1
fi
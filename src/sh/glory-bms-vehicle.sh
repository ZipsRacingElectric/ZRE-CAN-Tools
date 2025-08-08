#!/bin/bash

# Default to 1MBaud
BAUD=$1
if [[ $BAUD == "" ]]; then
	BAUD=1000000
fi

# Initialize the CAN device
DEVICE=$($ZRE_CANTOOLS_DIR/bin/init-can $BAUD $ZRE_CANTOOLS_DEV)
if [[ $? != 0 ]]; then
	exit $?
fi

# Start the application
$ZRE_CANTOOLS_DIR/bin/bms-tui $DEVICE $ZRE_CANTOOLS_DIR/config/zr25_glory/can_vehicle.dbc $ZRE_CANTOOLS_DIR/config/zr25_glory/bms_config.json
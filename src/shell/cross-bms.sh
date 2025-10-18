#!/bin/bash

# Arguments:
# - 1 - Device name (optional)

# Initialize the CAN device
DEVICE=$($ZRE_CANTOOLS_DIR/bin/can-init 1000000 $1)
if [ "$?" != 0 ]; then
	exit $?
fi

# Start the application
$ZRE_CANTOOLS_DIR/bin/bms-tui $DEVICE $ZRE_CANTOOLS_DIR/config/zre24_cross/can.dbc $ZRE_CANTOOLS_DIR/config/zre24_cross/bms_config.json
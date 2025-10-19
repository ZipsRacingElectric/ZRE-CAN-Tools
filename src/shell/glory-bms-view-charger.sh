#!/bin/bash

# Arguments:
# - 1 - Device name (optional)

# Initialize the CAN device
DEVICE=$($ZRE_CANTOOLS_DIR/bin/can-init 500000 $1)
if [ "$?" != 0 ]; then
	exit $?
fi

# Start the application
$ZRE_CANTOOLS_DIR/bin/bms-tui $DEVICE $ZRE_CANTOOLS_DIR/config/zr25_glory/can_vehicle.dbc $ZRE_CANTOOLS_DIR/config/zr25_glory/bms_config.json
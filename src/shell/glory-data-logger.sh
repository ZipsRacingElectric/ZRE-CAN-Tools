#!/bin/bash

# Arguments:
# - 1 - Device name (optional)

# Initialize the CAN device
DEVICE=$($ZRE_CANTOOLS_DIR/bin/can-init 1000000 $1)
if [ "$?" != 0 ]; then
	exit $?
fi

# Start the application
$ZRE_CANTOOLS_DIR/bin/can-mdf-logger $ZRE_CANTOOLS_LOGGING_DIR $ZRE_CANTOOLS_DIR/config/zr25_glory/logger_vehicle_config.json $DEVICE
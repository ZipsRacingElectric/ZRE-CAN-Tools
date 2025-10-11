#!/bin/bash

# Arguments:
# - 1 - Device name (optional)

# Initialize the CAN device
DEVICE=$($ZRE_CANTOOLS_DIR/bin/can-init 1000000 $1)
if [ "$?" != 0 ]; then
	exit $?
fi

# Start the application
$ZRE_CANTOOLS_DIR/bin/can-eeprom-cli $DEVICE $ZRE_CANTOOLS_DIR/config/zr25_glory/bms_config.json
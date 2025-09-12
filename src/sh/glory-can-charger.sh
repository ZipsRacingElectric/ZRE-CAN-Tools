#!/bin/sh

# Default to 500kBaud
BAUD=$1
if [ "$BAUD" = "" ]; then
	BAUD=500000
fi

# Initialize the CAN device
DEVICE=$($ZRE_CANTOOLS_DIR/bin/init-can $BAUD $ZRE_CANTOOLS_DEV)
if [ "$?" != 0 ]; then
	exit $?
fi

# Start the application
$ZRE_CANTOOLS_DIR/bin/can-dbc-tui $DEVICE $ZRE_CANTOOLS_DIR/config/zr25_glory/can_charger.dbc
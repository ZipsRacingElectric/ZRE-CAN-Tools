#!/bin/bash

# Default to 1MBaud
BAUD=$1
if [ "$BAUD" = "" ]; then
	BAUD=1000000
fi

# Initialize the CAN device
DEVICE=$($ZRE_CANTOOLS_DIR/bin/init-can $BAUD $ZRE_CANTOOLS_DEV)
if [ $? -ne 0 ]; then
	exit $?
fi

$ZRE_CANTOOLS_DIR/bin/can-eeprom-cli -r $DEVICE $ZRE_CANTOOLS_DIR/config/zr25_glory/vcu_config.json | octave $ZRE_CANTOOLS_DIR/bin/ls_steer_plot.m
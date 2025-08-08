#!/bin/bash

# Check a config was specified
if [[ $1 == "" ]]; then
	echo Must specify a driver config.
	exit -1
fi

CONFIG=$ZRE_CANTOOLS_DIR/config/drivers/$1.json

# Default to 1MBaud
BAUD=$2
if [[ $BAUD == "" ]]; then
	BAUD=1000000
fi

# Initialize the CAN device
DEVICE=$($ZRE_CANTOOLS_DIR/bin/init-can $BAUD $ZRE_CANTOOLS_DEV)
if [[ $? != 0 ]]; then
	exit $?
fi

# Start the application
$ZRE_CANTOOLS_DIR/bin/can-eeprom-cli -r=$DRIVER_CONFIG $DEVICE $ZRE_CANTOOLS_DIR/config/zr25_glory/vcu_config.json
#!/bin/bash

if [[ $1 == "" ]]; then
	echo Must specify a driver config.
	exit -1
fi

if [[ $2 == "" ]]; then
	. init-can 1000000
else
	. init-can $1
fi

DRIVER_CONFIG=$ZRE_CANTOOLS_DIR/config/drivers/$1.json

if [ -f "$DRIVER_CONFIG" ]; then
	echo Using driver config: \'$DRIVER_CONFIG\'
else
	echo Driver config \'$DRIVER_CONFIG\' does not exist.
	exit -1
fi

echo Using CAN device: $ZRE_CANTOOLS_DEV
$ZRE_CANTOOLS_DIR/bin/can-eeprom-cli -p=$DRIVER_CONFIG $ZRE_CANTOOLS_DEV $ZRE_CANTOOLS_DIR/config/zr25_glory/vcu_config.json
#!/bin/bash

if [[ $1 == "" ]]; then
	. init-can 1000000
else
	. init-can $1
fi

echo Using CAN device: $ZRE_CANTOOLS_DEV
$ZRE_CANTOOLS_DIR/bin/bms-tui $ZRE_CANTOOLS_DEV $ZRE_CANTOOLS_DIR/config/zr25_glory/can_vehicle.dbc $ZRE_CANTOOLS_DIR/config/zr25_glory/bms_config.json
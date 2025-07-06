#!/bin/bash

if [[ $1 == "" ]]; then
	. init-can 1000000
else
	. init-can $1
fi

echo Using CAN device: $ZRE_CANTOOLS_DEV
$ZRE_CANTOOLS_DIR/bin/can-dbc-tui $ZRE_CANTOOLS_DEV $ZRE_CANTOOLS_DIR/config/glory/can_vehicle.dbc
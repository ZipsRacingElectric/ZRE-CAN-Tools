#!/bin/bash

if [[ $1 == "" ]]; then
	. init-can 500000
else
	. init-can $1
fi

echo Using CAN device: $ZRE_CANTOOLS_DEV
$ZRE_CANTOOLS_DIR/bin/can-dbc-tui $ZRE_CANTOOLS_DEV $ZRE_CANTOOLS_DIR/config/zr25_glory/can_charger.dbc
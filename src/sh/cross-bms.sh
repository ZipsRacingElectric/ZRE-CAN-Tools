#!/bin/bash

if [[ $1 == "" ]]; then
	. init-can 1000000
else
	. init-can $1
fi

$ZRE_CANTOOLS_DIR/bin/bms-tui $ZRE_CANTOOLS_DEV $ZRE_CANTOOLS_DIR/config/cross/can.dbc $ZRE_CANTOOLS_DIR/config/cross/bms_config.json
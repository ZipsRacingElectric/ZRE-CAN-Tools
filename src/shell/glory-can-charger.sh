#!/bin/bash

# Arguments:
# - 1 - Device name (optional)

if [ "$1" == "" ]; then
	$ZRE_CANTOOLS_DIR/bin/can-dbc-tui "/dev/tty*@500000" $ZRE_CANTOOLS_DIR/config/zr25_glory/can_charger.dbc
else
	$ZRE_CANTOOLS_DIR/bin/can-dbc-tui "$1@500000" $ZRE_CANTOOLS_DIR/config/zr25_glory/can_charger.dbc
fi
#!/bin/bash

# Arguments:
# - 1 - Device name (optional)

if [ "$1" == "" ]; then
	$ZRE_CANTOOLS_DIR/bin/dashboard-gui $ZRE_CANTOOLS_DIR/config/zr26/charger/dashboard_gui.json "/dev/tty*@500000" $ZRE_CANTOOLS_DIR/config/zr26/charger/main.dbc
else
	$ZRE_CANTOOLS_DIR/bin/dashboard-gui $ZRE_CANTOOLS_DIR/config/zr26/charger/dashboard_gui.json "$1@500000" $ZRE_CANTOOLS_DIR/config/zr26/charger/main.dbc
fi

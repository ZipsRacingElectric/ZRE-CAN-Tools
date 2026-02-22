#!/bin/bash

# Arguments:
# - 1 - Device name (optional)

if [ "$1" == "" ]; then
	$ZRE_CANTOOLS_DIR/bin/dashboard-gui $ZRE_CANTOOLS_DIR/config/zr25/vehicle/dashboard_gui.json "/dev/tty*@1000000" $ZRE_CANTOOLS_DIR/config/zr25/vehicle/main.dbc
else
	$ZRE_CANTOOLS_DIR/bin/dashboard-gui $ZRE_CANTOOLS_DIR/config/zr25/vehicle/dashboard_gui.json "$1@1000000" $ZRE_CANTOOLS_DIR/config/zr25/vehicle/main.dbc
fi

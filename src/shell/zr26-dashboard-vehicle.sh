#!/bin/bash

# Arguments:
# - 1 - Device 0 name (optional)

if [ "$1" == "" ]; then
	$ZRE_CANTOOLS_DIR/bin/dashboard-gui $ZRE_CANTOOLS_DIR/config/zr26/vehicle/dashboard_gui.json "/dev/tty*@1000000" "/dev/tty*@1000000"
else
	$ZRE_CANTOOLS_DIR/bin/dashboard-gui $ZRE_CANTOOLS_DIR/config/zr26/vehicle/dashboard_gui.json "$1@1000000" "/dev/tty*@1000000"
fi

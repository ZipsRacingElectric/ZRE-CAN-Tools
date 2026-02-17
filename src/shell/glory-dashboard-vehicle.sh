#!/bin/bash

# Arguments:
# - 1 - Device name (optional)

if [ "$1" == "" ]; then
	$ZRE_CANTOOLS_DIR/bin/dashboard-gui $ZRE_CANTOOLS_DIR/config/zr25_glory/dashboard_vehicle_config.json "/dev/tty*@1000000" $ZRE_CANTOOLS_DIR/config/zr25_glory/can_vehicle.dbc
else
	$ZRE_CANTOOLS_DIR/bin/dashboard-gui $ZRE_CANTOOLS_DIR/config/zr25_glory/dashboard_vehicle_config.json "$1@1000000" $ZRE_CANTOOLS_DIR/config/zr25_glory/can_vehicle.dbc
fi

#!/bin/bash

# Arguments:
# - 1 - Device name (optional)

if [ "$1" == "" ]; then
	$ZRE_CANTOOLS_DIR/bin/bms-tui "/dev/tty*@1000000" $ZRE_CANTOOLS_DIR/config/zre24_cross/can.dbc $ZRE_CANTOOLS_DIR/config/zre24_cross/bms_config.json
else
	$ZRE_CANTOOLS_DIR/bin/bms-tui "$1@1000000" $ZRE_CANTOOLS_DIR/config/zre24_cross/can.dbc $ZRE_CANTOOLS_DIR/config/zre24_cross/bms_config.json
fi

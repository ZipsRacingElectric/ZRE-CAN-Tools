#!/bin/bash

# Arguments:
# - 1 - Device name (optional)

if [ "$1" == "" ]; then
	$ZRE_CANTOOLS_DIR/bin/can-eeprom-cli "/dev/tty*@500000" $ZRE_CANTOOLS_DIR/config/zr26/bms_config.json
else
	$ZRE_CANTOOLS_DIR/bin/can-eeprom-cli "$1@500000" $ZRE_CANTOOLS_DIR/config/zr26/bms_config.json
fi
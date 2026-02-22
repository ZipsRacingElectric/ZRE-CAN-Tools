#!/bin/bash

# Arguments:
# - 1 - Device name (optional)

if [ "$1" == "" ]; then
	$ZRE_CANTOOLS_DIR/bin/can-eeprom-cli "/dev/tty*@1000000" $ZRE_CANTOOLS_DIR/config/zr25/vcu_config.json $ZRE_CANTOOLS_DIR/config/zr25/bms_config.json $ZRE_CANTOOLS_DIR/config/zr25/drs_config.json
else
	$ZRE_CANTOOLS_DIR/bin/can-eeprom-cli "$1@1000000" $ZRE_CANTOOLS_DIR/config/zr25/vcu_config.json $ZRE_CANTOOLS_DIR/config/zr25/bms_config.json $ZRE_CANTOOLS_DIR/config/zr25/drs_config.json
fi
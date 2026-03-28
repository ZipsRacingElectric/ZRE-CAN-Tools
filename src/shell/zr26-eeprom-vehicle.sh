#!/bin/sh
$ZRE_CANTOOLS_DIR/bin/can-eeprom-cli "$@" "/dev/tty*@1000000" $ZRE_CANTOOLS_DIR/config/zr26/vcu_config.json $ZRE_CANTOOLS_DIR/config/zr26/bms_config.json $ZRE_CANTOOLS_DIR/config/zr26/drs_config.json

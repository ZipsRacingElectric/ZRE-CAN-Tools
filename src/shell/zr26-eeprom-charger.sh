#!/bin/bash
$ZRE_CANTOOLS_DIR/bin/can-eeprom-cli "$@" "/dev/tty*@500000" $ZRE_CANTOOLS_DIR/config/zr26/bms_config.json
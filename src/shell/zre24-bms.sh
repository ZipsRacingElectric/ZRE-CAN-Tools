#!/bin/bash
$ZRE_CANTOOLS_DIR/bin/bms-tui "$@" "/dev/tty*@1000000" $ZRE_CANTOOLS_DIR/config/zre24/main.dbc $ZRE_CANTOOLS_DIR/config/zre24/bms_config.json

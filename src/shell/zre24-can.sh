#!/bin/bash
$ZRE_CANTOOLS_DIR/bin/can-dbc-tui "$@" "/dev/tty*@1000000" $ZRE_CANTOOLS_DIR/config/zre24/main.dbc

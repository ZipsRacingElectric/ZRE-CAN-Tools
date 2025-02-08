#! bin/sh
octave src/tv_chatfield_dummy_prog/main.m | can-eeprom-cli -p can0 config/vcu_config.json
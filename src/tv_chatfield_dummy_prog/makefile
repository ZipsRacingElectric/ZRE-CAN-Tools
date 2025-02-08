BIN_DIR	:= ../../bin
BIN		:= $(BIN_DIR)/tv-dummy-prog

SRC 	:=	main.m

PROJECT_DIR := $(CURDIR)/../..

$(BIN): $(SRC)
	mkdir -p $(BIN_DIR)
	rm -f $(BIN)
	echo "#!/bin/bash" >> $(BIN)
	echo "octave $(CURDIR)/$(SRC) | can-eeprom-cli -p \$$@ $(PROJECT_DIR)/config/vcu_config.json" >> $(BIN)
	chmod a+x $(BIN)
# Directories
BIN_DIR := bin

# Libraries
CAN_LIB := $(BIN_DIR)/can.a
CJSON_LIB := $(BIN_DIR)/cjson.a

# Applications
CAN_DBC_CLI		:= $(BIN_DIR)/can-dbc-cli
CAN_DBC_TUI		:= $(BIN_DIR)/can-dbc-tui
CAN_EEPROM_CLI	:= $(BIN_DIR)/can-eeprom-cli
TV_DUMMY_PROG	:= $(BIN_DIR)/tv-dummy-prog
BMS_TUI			:= $(BIN_DIR)/bms-tui

all: $(CAN_DBC_CLI) $(CAN_DBC_TUI) $(CAN_EEPROM_CLI) $(TV_DUMMY_PROG) $(BMS_TUI)

$(CAN_LIB): FORCE
	make -C lib/can

$(CJSON_LIB): FORCE
	make -C lib/cjson

$(CAN_DBC_CLI): $(CAN_LIB) FORCE
	make -C src/can_dbc_cli

$(CAN_DBC_TUI): $(CAN_LIB) FORCE
	make -C src/can_dbc_tui

$(CAN_EEPROM_CLI): $(CAN_LIB) $(CJSON_LIB) FORCE
	make -C src/can_eeprom_cli

$(TV_DUMMY_PROG): FORCE
	make -C src/tv_chatfield_dummy_prog

$(BMS_TUI): FORCE
	make -C src/bms_tui

# Phony target, forces dependent targets to always be re-compiled
FORCE:

# Cleanup
clean:
	rm -r $(BIN_DIR)
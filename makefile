# Directories
BIN_DIR := bin

# Libraries
CAN_LIB := $(BIN_DIR)/can.a
CJSON_LIB := $(BIN_DIR)/cjson.a

# Applications
CAN_DBC_CLI := $(BIN_DIR)/can-dbc-cli
CAN_DBC_TUI := $(BIN_DIR)/can-dbc-tui
CAN_EEPROM_CLI := $(BIN_DIR)/can-eeprom-cli

all: $(CAN_DBC_CLI) $(CAN_DBC_TUI) $(CAN_EEPROM_CLI)

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

# Phony target, forces dependent targets to always be re-compiled
FORCE:

# Cleanup
clean:
	rm -r $(BIN_DIR)
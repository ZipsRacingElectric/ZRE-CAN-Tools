# Directories
BIN_DIR := bin

# Libraries
CAN_DEVICE_LIB := $(BIN_DIR)/can_device.a
CAN_DATABASE_LIB := $(BIN_DIR)/can_database.a
CAN_EEPROM_LIB := $(BIN_DIR)/can_eeprom.a
CJSON_LIB := $(BIN_DIR)/cjson.a
BMS_LIB := $(BIN_DIR)/bms.a

# Applications
CAN_DBC_CLI		:= $(BIN_DIR)/can-dbc-cli
CAN_DBC_TUI		:= $(BIN_DIR)/can-dbc-tui
CAN_EEPROM_CLI	:= $(BIN_DIR)/can-eeprom-cli
BMS_TUI			:= $(BIN_DIR)/bms-tui

all: $(CAN_DBC_CLI) $(CAN_DBC_TUI) $(CAN_EEPROM_CLI) $(TV_DUMMY_PROG) $(BMS_TUI) shell

# Libraries
$(CAN_DEVICE_LIB): FORCE
	make -C lib/can_device

$(CAN_DATABASE_LIB): FORCE
	make -C lib/can_database

$(CAN_EEPROM_LIB): $(CAN_DEVICE_LIB) $(CJSON_LIB) FORCE
	make -C lib/can_eeprom

$(CJSON_LIB): FORCE
	make -C lib/cjson

$(BMS_LIB): $(CAN_DEVICE_LIB) $(CAN_DATABASE_LIB) $(CJSON_LIB) FORCE
	make -C lib/bms

# Applications
$(CAN_DBC_CLI): $(CAN_DEVICE_LIB) $(CAN_DATABASE_LIB) FORCE
	make -C src/can_dbc_cli

$(CAN_DBC_TUI): $(CAN_DEVICE_LIB) $(CAN_DATABASE_LIB) FORCE
	make -C src/can_dbc_tui

$(CAN_EEPROM_CLI): $(CAN_EEPROM_LIB) FORCE
	make -C src/can_eeprom_cli

$(TV_DUMMY_PROG): FORCE
	make -C src/tv_chatfield_dummy_prog

$(BMS_TUI): $(CAN_DEVICE_LIB) $(CAN_DATABASE_LIB) $(BMS_LIB) FORCE
	make -C src/bms_tui

shell: FORCE
	make -C src/shell

# Phony target, forces dependent targets to always be re-compiled
FORCE:

# Cleanup
clean:
	rm -r $(BIN_DIR)
# Directories
BIN_DIR := bin
LIB_DIR := $(BIN_DIR)/lib

# Libraries
LIB := $(LIB_DIR)/lib.a
CAN_DEVICE_LIB := $(LIB_DIR)/can_device.a
CAN_DATABASE_LIB := $(LIB_DIR)/can_database.a
CAN_EEPROM_LIB := $(LIB_DIR)/can_eeprom.a
CJSON_LIB := $(LIB_DIR)/cjson.a
BMS_LIB := $(LIB_DIR)/bms.a
SERIAL_CAN_LIB := $(LIB_DIR)/serial_can.a

# Applications
CAN_DEV_CLI		:= $(BIN_DIR)/can-dev-cli
CAN_DBC_CLI		:= $(BIN_DIR)/can-dbc-cli
CAN_DBC_TUI		:= $(BIN_DIR)/can-dbc-tui
CAN_EEPROM_CLI	:= $(BIN_DIR)/can-eeprom-cli
BMS_TUI			:= $(BIN_DIR)/bms-tui

all: $(CAN_DEV_CLI) $(CAN_DBC_CLI) $(CAN_DBC_TUI) $(CAN_EEPROM_CLI) $(BMS_TUI) shell

# Libraries
$(LIB): FORCE
	make -C lib

$(SERIAL_CAN_LIB): $(LIB) FORCE
	make -B -C lib/serial_can

$(CAN_DEVICE_LIB): $(LIB) $(SERIAL_CAN_LIB) FORCE
	make -C lib/can_device

$(CAN_DATABASE_LIB): $(LIB) FORCE
	make -C lib/can_database

$(CAN_EEPROM_LIB): $(LIB) $(CAN_DEVICE_LIB) $(SERIAL_CAN_LIB) $(CJSON_LIB) FORCE
	make -C lib/can_eeprom

$(CJSON_LIB): $(LIB) FORCE
	make -C lib/cjson

$(BMS_LIB): $(LIB) $(CAN_DEVICE_LIB) $(SERIAL_CAN_LIB) $(CAN_DATABASE_LIB) $(CJSON_LIB) FORCE
	make -C lib/bms

# Applications
$(CAN_DEV_CLI): $(CAN_DEVICE_LIB) $(SERIAL_CAN_LIB) FORCE
	make -C src/can_dev_cli

$(CAN_DBC_CLI): $(CAN_DEVICE_LIB) $(SERIAL_CAN_LIB) $(CAN_DATABASE_LIB) FORCE
	make -C src/can_dbc_cli

$(CAN_DBC_TUI): $(CAN_DEVICE_LIB) $(SERIAL_CAN_LIB) $(CAN_DATABASE_LIB) FORCE
	make -C src/can_dbc_tui

$(CAN_EEPROM_CLI): $(CAN_DEVICE_LIB) $(SERIAL_CAN_LIB) $(CAN_EEPROM_LIB) FORCE
	make -C src/can_eeprom_cli

$(BMS_TUI): $(CAN_DEVICE_LIB) $(SERIAL_CAN_LIB) $(CAN_DATABASE_LIB) $(BMS_LIB) FORCE
	make -C src/bms_tui

shell: FORCE
	make -C src/shell

# Phony target, forces dependent targets to always be re-compiled
FORCE:

# Cleanup
clean:
	rm -r $(BIN_DIR)
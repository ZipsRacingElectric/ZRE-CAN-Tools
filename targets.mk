$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(BIN_LIB_DIR): $(BIN_DIR)
	mkdir -p $(BIN_LIB_DIR)

$(BIN_OBJ_DIR): $(BIN_DIR)
	mkdir -p $(BIN_OBJ_DIR)

COMMON_LIB			:= $(BIN_LIB_DIR)/libcommon.a
CAN_DEVICE_LIB		:= $(BIN_LIB_DIR)/libcan_device.a
CAN_DATABASE_LIB	:= $(BIN_LIB_DIR)/libcan_database.a
CAN_EEPROM_LIB		:= $(BIN_LIB_DIR)/libcan_eeprom.a
CJSON_LIB			:= $(BIN_LIB_DIR)/libcjson.a
BMS_LIB				:= $(BIN_LIB_DIR)/libbms.a
SERIAL_CAN_LIB		:= $(BIN_LIB_DIR)/libserial_can.a

$(COMMON_LIB): FORCE
	make -C $(LIB_DIR) $@

# TODO(Barach): Needs -B?
$(SERIAL_CAN_LIB): FORCE
	make -B -C $(LIB_DIR)/serial_can

$(CAN_DEVICE_LIB): FORCE
	make -C $(LIB_DIR)/can_device $@

$(CAN_DATABASE_LIB): FORCE
	make -C $(LIB_DIR)/can_database $@

$(CAN_EEPROM_LIB): FORCE
	make -C $(LIB_DIR)/can_eeprom $@

$(CJSON_LIB): FORCE
	make -C $(LIB_DIR)/cjson $@

$(BMS_LIB): FORCE
	make -C $(LIB_DIR)/bms $@

FORCE:
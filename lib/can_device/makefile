include $(TARGETS_MK)

SRC := $(wildcard *.c)
OBJ := $(patsubst %.c, $(BIN_OBJ_DIR)/%.o, $(SRC))

$(CAN_DEVICE_LIB): $(OBJ)
	ar rvs $@ $(OBJ)

$(BIN_OBJ_DIR)/%.o: %.c %.h $(SERIAL_CAN_LIB) $(BIN_LIB_DIR) $(BIN_OBJ_DIR)
	gcc $(CC_FLAGS) -I $(BIN_DIR)/include -c $< -o $@
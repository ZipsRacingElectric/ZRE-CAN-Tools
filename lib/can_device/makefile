LIB_CAN_DEVICE := $(BIN_LIB_DIR)/libcan_device.a

private DIR := $(LIB_DIR)/can_device
private BIN := $(LIB_CAN_DEVICE)

private SRC := $(wildcard $(DIR)/*.c)
private OBJ := $(patsubst $(DIR)/%.c, $(BIN_OBJ_DIR)/%.o, $(SRC))

$(BIN): $(OBJ)
	mkdir -p $(BIN_LIB_DIR)
	ar rvs $@ $^

$(BIN_OBJ_DIR)/%.o: $(DIR)/%.c $(DIR)/%.h $(BIN_LIB_DIR)/serial_can.a
	mkdir -p $(BIN_OBJ_DIR)
	gcc $(CFLAGS) -I $(BIN_DIR)/include -c $< -o $@

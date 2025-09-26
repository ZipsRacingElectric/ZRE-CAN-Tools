private BIN := $(BIN_DIR)/can-dev-cli
private DIR := $(SRC_DIR)/can_dev_cli
private SRC := $(DIR)/main.c

ALL += $(BIN)

# Note libraries must be in reverse order of dependencies,
# that is a dependency must be placed after its dependents.
private LIB :=				\
	$(LIB_CAN_DEVICE)		\
	$(LIB_SERIAL_CAN)		\
	$(LIB_COMMON)

$(BIN): $(SRC) $(LIB)
	mkdir -p $(BIN_DIR)
	gcc $^ $(CFLAGS) -o $@

BIN := $(BIN_DIR)/can-dev-cli

SRC := main.c

include $(TARGETS_MK)

# Note libraries must be in reverse order of dependencies, that is a dependency must be placed after its dependents.
LIB :=						\
	$(CAN_DEVICE_LIB)		\
	$(SERIAL_CAN_LIB)		\
	$(COMMON_LIB)

$(BIN): $(SRC) $(LIB)
	gcc $(SRC) $(CC_FLAGS) $(LIB) -o $(BIN)
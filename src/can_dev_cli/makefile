CC_FLAGS := -Wall -Wextra -g

BIN_DIR	:= ../../bin
LIB_DIR := $(BIN_DIR)/lib
BIN		:= $(BIN_DIR)/can-dev-cli

SRC 	:= main.c

# Note libraries must be in reverse order of dependencies, that is a dependency must be placed after its dependents.
LIB		:=						\
	$(LIB_DIR)/can_device.a		\
	$(LIB_DIR)/serial_can.a		\
	$(LIB_DIR)/lib.a

$(BIN): $(SRC) $(LIB)
	mkdir -p $(BIN_DIR)
	gcc $(SRC) $(CC_FLAGS) $(LIB) -I../../lib/ -o $(BIN)
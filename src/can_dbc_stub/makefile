ROOT_DIR := ../..
include $(ROOT_DIR)/include.mk

BIN := $(BIN_DIR)/can-dbc-stub
SRC := main.c

# Note libraries must be in reverse order of dependencies, that is a dependency
# must be placed after its dependents.
LIB :=						\
	$(LIB_CAN_DEVICE)		\
	$(LIB_CAN_DATABASE)		\
	$(LIB_SERIAL_CAN)		\
	$(LIB_COMMON)

# Linking against curses, so include its flags
LIBFLAGS += $(LIB_CURSES_LIBFLAGS)

$(BIN): $(SRC) $(LIB)
	mkdir -p $(BIN_DIR)
	gcc $^ $(CFLAGS) -o $@ $(LIBFLAGS)
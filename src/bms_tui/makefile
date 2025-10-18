ROOT_DIR := ../..
include $(ROOT_DIR)/include.mk

BIN := $(BIN_DIR)/bms-tui
SRC := main.c

# Note libraries must be in reverse order of dependencies, that is a dependency
# must be placed after its dependents.
LIB :=						\
	$(LIB_BMS)				\
	$(LIB_CJSON)			\
	$(LIB_CAN_DATABASE)		\
	$(LIB_CAN_DEVICE)		\
	$(LIB_SERIAL_CAN)		\
	$(LIB_COMMON)

$(BIN): $(SRC) $(LIB)
	mkdir -p $(BIN_DIR)
	gcc $^ $(CFLAGS) -o $@ -lncursesw
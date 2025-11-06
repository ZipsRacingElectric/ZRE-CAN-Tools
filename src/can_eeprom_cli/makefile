ROOT_DIR := ../..
include $(ROOT_DIR)/include.mk

BIN := $(BIN_DIR)/can-eeprom-cli
SRC := main.c

# Note libraries must be in reverse order of dependencies, that is a dependency
# must be placed after its dependents.
LIB :=						\
	$(LIB_CAN_DEVICE)		\
	$(LIB_SERIAL_CAN)		\
	$(LIB_CJSON)			\
	$(LIB_CAN_EEPROM)		\
	$(LIB_COMMON)

$(BIN): $(SRC) $(LIB)
	mkdir -p $(BIN_DIR)
	gcc $^ $(CFLAGS) -o $@ -lsetupapi -luuid
# TODO(DiBacco): find a better approach for including linker flags for the setupapi & libuuid libraries.
# Ideally, this solution would provide os specificity and allow the linkage to be done centrally.

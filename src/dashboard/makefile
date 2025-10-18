ROOT_DIR := ../..
include $(ROOT_DIR)/include.mk

BIN := $(BIN_DIR)/dashboard
SRC := main.c

CFLAGS += $(shell pkg-config --cflags gtk4)
LIBFLAGS += $(shell pkg-config --libs gtk4)

all: $(BIN) clangd-file

CLANGD_FILE := ./.vscode/compile_commands.json
include $(ROOT_DIR)/clangd.mk

# Note libraries must be in reverse order of dependencies, that is a dependency
# must be placed after its dependents.
LIB :=						\
	$(LIB_CAN_DATABASE)		\
	$(LIB_CAN_DEVICE)		\
	$(LIB_SERIAL_CAN)		\
	$(LIB_COMMON)

$(BIN): $(SRC) $(LIB)
	mkdir -p $(BIN_DIR)
	gcc $^ $(CFLAGS) -o $@ $(LIBFLAGS)

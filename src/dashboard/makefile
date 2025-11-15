ROOT_DIR := ../..
include $(ROOT_DIR)/include.mk

BIN := $(BIN_DIR)/dashboard
SRC := main.c

# Compiling and linking against GTK, so include its flags
CFLAGS += $(LIB_GTK_CFLAGS)
LIBFLAGS += $(LIB_GTK_LIBFLAGS)

# Note libraries must be in reverse order of dependencies, that is a dependency
# must be placed after its dependents.
LIB :=						\
	$(LIB_CAN_DATABASE)		\
	$(LIB_CAN_DEVICE)		\
	$(LIB_SERIAL_CAN)		\
	$(LIB_COMMON)

all: $(BIN)

$(BIN): $(SRC) $(LIB)
	mkdir -p $(BIN_DIR)
	gcc $^ $(CFLAGS) -o $@ $(LIBFLAGS)

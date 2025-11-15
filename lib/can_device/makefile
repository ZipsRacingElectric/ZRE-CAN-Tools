ROOT_DIR := ../..
include $(ROOT_DIR)/include.mk

BIN := $(LIB_CAN_DEVICE)
SRC := $(wildcard *.c)
OBJ := $(patsubst %.c, $(OBJ_BIN_DIR)/%.o, $(SRC))

# Compiling against libserial_can, so include its flags
CFLAGS += $(LIB_SERIAL_CAN_CFLAGS)

$(BIN): $(OBJ)
	mkdir -p $(LIB_BIN_DIR)
	ar rvs $@ $^

$(OBJ_BIN_DIR)/%.o: %.c %.h
	mkdir -p $(OBJ_BIN_DIR)
	gcc $(CFLAGS) -c $< -o $@ $(LIBFLAGS)

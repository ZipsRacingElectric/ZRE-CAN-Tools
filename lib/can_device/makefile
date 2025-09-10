CC_FLAGS := -Wall -Wextra -g

BIN_DIR := ../../bin
OBJ_DIR := $(BIN_DIR)/obj
LIB_DIR := $(BIN_DIR)/lib

BIN := $(LIB_DIR)/can_device.a

SRC := $(wildcard *.c)
OBJ := $(patsubst %.c, $(OBJ_DIR)/%.o, $(SRC))

$(BIN): $(OBJ)
	mkdir -p $(LIB_DIR)
	ar rvs $(BIN) $^

$(OBJ_DIR)/%.o: %.c %.h
	mkdir -p $(OBJ_DIR)
	gcc $(CC_FLAGS) -I.. -I$(BIN_DIR)/include -c $< -o $@
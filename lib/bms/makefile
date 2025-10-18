ROOT_DIR := ../..
include $(ROOT_DIR)/include.mk

BIN := $(LIB_BMS)
SRC := $(wildcard *.c)
OBJ := $(patsubst %.c, $(OBJ_BIN_DIR)/%.o, $(SRC))

$(BIN): $(OBJ)
	mkdir -p $(LIB_BIN_DIR)
	ar rvs $@ $^

$(OBJ_BIN_DIR)/%.o: %.c %.h
	mkdir -p $(OBJ_BIN_DIR)
	gcc $(CFLAGS) -I $(BIN_DIR)/include -c $< -o $@

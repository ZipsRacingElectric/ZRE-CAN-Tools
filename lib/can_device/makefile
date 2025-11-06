ROOT_DIR := ../..
include $(ROOT_DIR)/include.mk

BIN := $(LIB_CAN_DEVICE)
SRC := $(wildcard *.c)
OBJ := $(patsubst %.c, $(OBJ_BIN_DIR)/%.o, $(SRC))

$(BIN): $(OBJ)
	mkdir -p $(LIB_BIN_DIR)
	ar rvs $@ $^

$(OBJ_BIN_DIR)/%.o: %.c %.h
	mkdir -p $(OBJ_BIN_DIR)
	gcc $(CFLAGS) -I $(BIN_DIR)/include -c $< -o $@ -lsetupapi -luuid
# TODO(DiBacco): find a better approach for including linker flags for the setupapi & libuuid libraries.
# Ideally, this solution would provide os specificity and allow the linkage to be done centrally.
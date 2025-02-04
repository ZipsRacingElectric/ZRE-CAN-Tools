CC_FLAGS := -Wall -Wextra

BIN_DIR := ../../bin
OBJ_DIR := $(BIN_DIR)/obj

BIN := $(BIN_DIR)/cjson.a

SRC := $(wildcard *.c)
OBJ := $(patsubst %.c, $(OBJ_DIR)/%.o, $(SRC))

$(BIN): $(OBJ)
	ar rvs $(BIN) $^

$(OBJ_DIR)/%.o: %.c %.h
	mkdir -p $(OBJ_DIR)
	gcc $(CC_FLAGS) -I.. -c $< -o $@
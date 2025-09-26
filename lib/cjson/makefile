LIB_CJSON := $(BIN_LIB_DIR)/libcjson.a

private DIR := $(LIB_DIR)/cjson
private BIN := $(LIB_CJSON)

private SRC := $(wildcard $(DIR)/*.c)
private OBJ := $(patsubst $(DIR)/%.c, $(BIN_OBJ_DIR)/%.o, $(SRC))

$(BIN): $(OBJ)
	mkdir -p $(BIN_LIB_DIR)
	ar rvs $@ $^

$(BIN_OBJ_DIR)/%.o: $(DIR)/%.c $(DIR)/%.h
	mkdir -p $(BIN_OBJ_DIR)
	gcc $(CFLAGS) -c $< -o $@
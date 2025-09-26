LIB_COMMON := $(BIN_LIB_DIR)/libcommon.a

private DIR := $(LIB_DIR)
private BIN := $(LIB_COMMON)

private SRC := $(wildcard $(DIR)/*.c)
private OBJ := $(patsubst $(DIR)/%.c, $(BIN_OBJ_DIR)/%.o, $(SRC))

$(BIN): $(OBJ)
	mkdir -p $(BIN_LIB_DIR)
	ar rvs $@ $^

$(BIN_OBJ_DIR)/%.o: $(DIR)/%.c $(DIR)/%.h
	mkdir -p $(BIN_OBJ_DIR)
	gcc $(CFLAGS) -c $< -o $@

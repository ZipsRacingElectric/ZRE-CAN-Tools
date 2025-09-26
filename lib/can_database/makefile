LIB_CAN_DATABASE := $(BIN_LIB_DIR)/libcan_database.a

private DIR := $(LIB_DIR)/can_database
private BIN := $(LIB_CAN_DATABASE)

private SRC := $(wildcard $(DIR)/*.c)
private OBJ := $(patsubst $(DIR)/%.c, $(BIN_OBJ_DIR)/%.o, $(SRC))

$(BIN): $(OBJ)
	mkdir -p $(BIN_LIB_DIR)
	ar rvs $@ $^

$(BIN_OBJ_DIR)/%.o: $(DIR)/%.c $(DIR)/%.h
	mkdir -p $(BIN_OBJ_DIR)
	gcc $(CFLAGS) -c $< -o $@
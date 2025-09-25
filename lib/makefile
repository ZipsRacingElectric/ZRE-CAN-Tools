include $(TARGETS_MK)

SRC := $(wildcard *.c)
OBJ := $(patsubst %.c, $(BIN_OBJ_DIR)/%.o, $(SRC))

$(COMMON_LIB): $(OBJ)
	ar rvs $@ $(OBJ)

$(BIN_OBJ_DIR)/%.o: %.c %.h $(BIN_LIB_DIR) $(BIN_OBJ_DIR)
	gcc $(CC_FLAGS) -c $< -o $@
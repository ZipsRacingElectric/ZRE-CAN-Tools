private BIN := $(BIN_DIR)/bms-tui
private DIR := $(SRC_DIR)/bms_tui
private SRC := $(DIR)/main.c

ALL += $(BIN)

# Note libraries must be in reverse order of dependencies, that is a dependency
# must be placed after its dependents.
private LIB :=				\
	$(LIB_BMS)				\
	$(LIB_CJSON)			\
	$(LIB_CAN_DATABASE)		\
	$(LIB_COMMON)

$(BIN): $(SRC) $(LIB)
	mkdir -p $(BIN_DIR)
	gcc $^ $(CFLAGS) -o $@ -lncursesw
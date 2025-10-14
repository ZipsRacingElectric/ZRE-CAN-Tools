ROOT_DIR := ../..
include $(ROOT_DIR)/include.mk

BIN := $(BIN_DIR)/mdf-stub
SRC := main.c

# Note libraries must be in reverse order of dependencies, that is a dependency
# must be placed after its dependents.
LIB :=						\
	$(LIB_MDF)				\
	$(LIB_COMMON)

$(BIN): $(SRC) $(LIB)
	mkdir -p $(BIN_DIR)
	gcc $^ $(CFLAGS) -o $@

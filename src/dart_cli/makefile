ROOT_DIR := ../..
include $(ROOT_DIR)/include.mk

BIN := $(BIN_DIR)/dart-cli
SRC := main.c

# Note libraries must be in reverse order of dependencies, that is a dependency
# must be placed after its dependents.
LIB :=						\
	$(LIB_COMMON)

$(BIN): $(SRC) $(LIB)
	mkdir -p $(BIN_DIR)
	gcc $^ $(CFLAGS) -o $@ $(LIBFLAGS)

	cp -r keys $(BIN_DIR)/
	chmod 600 $(BIN_DIR)/keys/id_rsa
	chmod 644 $(BIN_DIR)/keys/id_rsa.pub
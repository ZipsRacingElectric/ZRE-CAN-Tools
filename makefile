export ROOT_DIR		= $(CURDIR)
export LIB_DIR		= $(ROOT_DIR)/lib
export BIN_DIR		= $(ROOT_DIR)/bin
export BIN_LIB_DIR	= $(BIN_DIR)/lib
export BIN_OBJ_DIR	= $(BIN_DIR)/obj

export TARGETS_MK	= $(ROOT_DIR)/targets.mk

export CC_FLAGS = -Wall -Wextra -g -I $(LIB_DIR)

all: $(BIN_DIR)/can-dev-cli

$(BIN_DIR)/can-dev-cli: FORCE
	$(MAKE) -C src/can_dev_cli/ $@

clean:
	rm -rf $(BIN_DIR)

include $(TARGETS_MK)
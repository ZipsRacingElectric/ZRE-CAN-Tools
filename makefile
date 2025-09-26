# Project Structure -----------------------------------------------------------

ROOT_DIR	:= .
SRC_DIR		:= $(ROOT_DIR)/src
LIB_DIR		:= $(ROOT_DIR)/lib
BIN_DIR		:= $(ROOT_DIR)/bin
BIN_LIB_DIR	:= $(BIN_DIR)/lib
BIN_OBJ_DIR	:= $(BIN_DIR)/obj

# Compilation -----------------------------------------------------------------

CFLAGS := -Wall -Wextra -g -I $(LIB_DIR) -lm

# Libraries -------------------------------------------------------------------

# Note that these are in order by dependency. The top-most is least dependent
# and the bottom-most is most dependent.

include lib/makefile

LIB_SERIAL_CAN := $(BIN_LIB_DIR)/libserial_can.a
$(LIB_SERIAL_CAN):
	make -B -C $(LIB_DIR)/serial_can/

include lib/can_device/makefile
include lib/can_database/makefile
include lib/cjson/makefile
include lib/can_eeprom/makefile
include lib/bms/makefile

# Applications ----------------------------------------------------------------

include src/bms_tui/makefile
include src/can_dev_cli/makefile

# Make Targets ----------------------------------------------------------------

.DEFAULT_GOAL := all
all: $(ALL)

clean:
	rm -rf $(BIN_DIR)
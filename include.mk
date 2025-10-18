# Project Structure -----------------------------------------------------------

SRC_DIR		:= $(ROOT_DIR)/src
LIB_DIR		:= $(ROOT_DIR)/lib
BIN_DIR		:= $(ROOT_DIR)/bin
LIB_BIN_DIR	:= $(BIN_DIR)/lib
OBJ_BIN_DIR	:= $(BIN_DIR)/obj

CONFIG_DIR	:= $(ROOT_DIR)/config
DOC_DIR		:= $(ROOT_DIR)/doc

# Library Definitions ---------------------------------------------------------

LIB_COMMON 			:= $(LIB_BIN_DIR)/libcommon.a
LIB_SERIAL_CAN		:= $(LIB_BIN_DIR)/libserial_can.a
LIB_CAN_DEVICE 		:= $(LIB_BIN_DIR)/libcan_device.a
LIB_CAN_DATABASE	:= $(LIB_BIN_DIR)/libcan_database.a
LIB_CJSON			:= $(LIB_BIN_DIR)/libcjson.a
LIB_CAN_EEPROM		:= $(LIB_BIN_DIR)/libcan_eeprom.a
LIB_BMS				:= $(LIB_BIN_DIR)/libbms.a
LIB_MDF				:= $(LIB_BIN_DIR)/libmdf.a

# Note that serial_can is not included in this list. This is because serial_can
# shouldn't be included as a PHONY target (see root makefile) or else it will
# be recompiled every time.
LIBS :=					\
	$(LIB_COMMON)		\
	$(LIB_CAN_DEVICE)	\
	$(LIB_CAN_DATABASE)	\
	$(LIB_CJSON)		\
	$(LIB_CAN_EEPROM)	\
	$(LIB_BMS)			\
	$(LIB_MDF)

# Compilation -----------------------------------------------------------------

# TODO(Barach): "-lm" should be in libflags, not CFLAGS
CFLAGS := -Wall -Wextra -g -I $(LIB_DIR) -lm

# Operating System Detection --------------------------------------------------

# - This is only used for tagging releases, everything in here should work in
#   both linux and MSYS2.
ifeq ($(OS),Windows_NT)
	DETECTED_OS := windows
else
	DETECTED_OS := $(shell uname | tr '[:upper:]' '[:lower:]')
endif
# Project Structure -----------------------------------------------------------

SRC_DIR		:= $(ROOT_DIR)/src
LIB_DIR		:= $(ROOT_DIR)/lib
BIN_DIR		:= $(ROOT_DIR)/bin
LIB_BIN_DIR	:= $(BIN_DIR)/lib
OBJ_BIN_DIR	:= $(BIN_DIR)/obj

CONFIG_DIR	:= $(ROOT_DIR)/config
DOC_DIR		:= $(ROOT_DIR)/doc
FONT_DIR	:= $(ROOT_DIR)/font

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

# Operating System Detection --------------------------------------------------

ifeq ($(OS),Windows_NT)
	OS_TYPE := windows
else
	OS_TYPE := $(shell uname | tr '[:upper:]' '[:lower:]')
endif

ARCH_TYPE := $(shell uname -m)

# Release Version -------------------------------------------------------------

VERSION_NUMBER := $(shell date +%Y.%m.%d)
VERSION_FULL := zre_cantools_$(OS_TYPE)_$(ARCH_TYPE)_$(VERSION_NUMBER)

# Compilation -----------------------------------------------------------------

# Compilation flags to use for compiling all applications and libraries
CFLAGS := -std=gnu11 -fno-strict-aliasing -Wall -Wextra -g					\
	-I $(LIB_DIR)															\
	-D ZRE_CANTOOLS_OS_$(OS_TYPE)											\
	-D ZRE_CANTOOLS_OS=\"$(OS_TYPE)\"										\
	-D ZRE_CANTOOLS_ARCH=\"$(ARCH_TYPE)\"									\
	-D ZRE_CANTOOLS_VERSION_NUMBER=\"$(VERSION_NUMBER)\"					\
	-D ZRE_CANTOOLS_VERSION_FULL=\"$(VERSION_FULL)\"						\
	-D ZRE_CANTOOLS_NAME=\"zre_cantools\"

# Linker flags to use for linking all applications and libraries
LIBFLAGS := -lm

# Library-Specific Flags ------------------------------------------------------

# Flags for using libserial_can
# - Note: for using libcan_device, this is not needed.
LIB_SERIAL_CAN_CFLAGS := -I $(BIN_DIR)/include

# Flags for using the Curses library
ifeq ($(OS),Windows_NT)
	LIB_CURSES_LIBFLAGS := -lncursesw
else
	LIB_CURSES_CFLAGS := $(shell pkg-config --cflags ncurses)
	LIB_CURSES_LIBFLAGS := $(shell pkg-config --libs ncurses)
endif

# Flags for using the GTK library
LIB_GTK_CFLAGS := $(shell pkg-config --cflags gtk4)
LIB_GTK_LIBFLAGS := $(shell pkg-config --libs gtk4)

# File Paths ------------------------------------------------------------------

ifeq ($(OS_TYPE),linux)
# 'realpath' gets the absolute path of the root directory, 'sed' escapes all spaces.
	ABS_ROOT_DIR_SAFE := $(shell realpath $(ROOT_DIR) | sed 's/ /\\ /g')
else
# 'realpath' gets the absolute path of the root directory, 'cygpath' converts this into a Windows path.
# First 'sed' escapes all '\' characters, second 'sed' escapes all spaces.
	ABS_ROOT_DIR_SAFE := $(shell cygpath -w $(shell realpath $(ROOT_DIR)) | sed 's/\\/\\\\/g' | sed 's/ /\\ /g')
endif

ifeq ($(OS_TYPE),linux)
# 'which gcc' gets the absolute path of the gcc binary, 'sed' escapes all spaces.
	GCC_BIN_SAFE := $(shell which gcc | sed 's/ /\\ /g')
else
# 'which gcc' gets the absolute path of the gcc binary, 'cygpath' converts this into a Windows path.
# First 'sed' escapes all '\' characters, second 'sed' escapes all spaces.
	GCC_BIN_SAFE := $(shell cygpath -w $(shell which gcc) | sed 's/\\/\\\\/g' | sed 's/ /\\ /g')
endif
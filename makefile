# Operating System Detection
# - This is only used for tagging releases, everything in here should work in both linux and MSYS2.
ifeq ($(OS),Windows_NT)
	# Windows_NT on XP, 2000, 7, Vista, 10...
	DETECTED_OS := windows
else
	# Same as "uname -s"
	DETECTED_OS := $(shell uname | tr '[:upper:]' '[:lower:]')
endif

# Project directories
BIN_DIR := bin
LIB_DIR := $(BIN_DIR)/lib
CONFIG_DIR := config
RELEASE_DIR := release/zre_cantools_$(DETECTED_OS)_$(shell date +%Y.%m.%d)
DOC_DIR := doc

# Libraries
LIB := $(LIB_DIR)/lib.a
CAN_DEVICE_LIB := $(LIB_DIR)/can_device.a
CAN_DATABASE_LIB := $(LIB_DIR)/can_database.a
CAN_EEPROM_LIB := $(LIB_DIR)/can_eeprom.a
CJSON_LIB := $(LIB_DIR)/cjson.a
BMS_LIB := $(LIB_DIR)/bms.a
SERIAL_CAN_LIB := $(LIB_DIR)/serial_can.a

# Applications
CAN_DEV_CLI		:= $(BIN_DIR)/can-dev-cli
CAN_DBC_CLI		:= $(BIN_DIR)/can-dbc-cli
CAN_DBC_TUI		:= $(BIN_DIR)/can-dbc-tui
CAN_EEPROM_CLI	:= $(BIN_DIR)/can-eeprom-cli
BMS_TUI			:= $(BIN_DIR)/bms-tui

# Installer script and readme file
ifeq ($(DETECTED_OS), windows)
	INSTALLER := install.bat
	UNINSTALLER := uninstall.bat
else
	INSTALLER := install.sh
	UNINSTALLER := uninstall.sh
endif
RELEASE_README := doc/readme_release.txt

# Command for copying the MSYS2 UCRT binaries into a release
ifeq ($(DETECTED_OS), windows)
	ifeq ($(MSYS_BIN), "")
		MSYS_UCRT_COPY_CMD := echo Error: MSYS_BIN environment variable is not declared! && exit -1
	else
		MSYS_UCRT_COPY_CMD := cp -r $(MSYS_BIN) $(RELEASE_DIR)/
	endif
endif

all: $(CAN_DEV_CLI) $(CAN_DBC_CLI) $(CAN_DBC_TUI) $(CAN_EEPROM_CLI) $(BMS_TUI) sh bat plot

# Libraries
$(LIB): FORCE
	make -C lib

$(SERIAL_CAN_LIB): $(LIB) FORCE
	make -B -C lib/serial_can

$(CAN_DEVICE_LIB): $(LIB) $(SERIAL_CAN_LIB) FORCE
	make -C lib/can_device

$(CAN_DATABASE_LIB): $(LIB) FORCE
	make -C lib/can_database

$(CAN_EEPROM_LIB): $(LIB) $(CAN_DEVICE_LIB) $(SERIAL_CAN_LIB) $(CJSON_LIB) FORCE
	make -C lib/can_eeprom

$(CJSON_LIB): $(LIB) FORCE
	make -C lib/cjson

$(BMS_LIB): $(LIB) $(CAN_DEVICE_LIB) $(SERIAL_CAN_LIB) $(CAN_DATABASE_LIB) $(CJSON_LIB) FORCE
	make -C lib/bms

# Applications
$(CAN_DEV_CLI): $(CAN_DEVICE_LIB) $(SERIAL_CAN_LIB) FORCE
	make -C src/can_dev_cli

$(CAN_DBC_CLI): $(CAN_DEVICE_LIB) $(SERIAL_CAN_LIB) $(CAN_DATABASE_LIB) FORCE
	make -C src/can_dbc_cli

$(CAN_DBC_TUI): $(CAN_DEVICE_LIB) $(SERIAL_CAN_LIB) $(CAN_DATABASE_LIB) FORCE
	make -C src/can_dbc_tui

$(CAN_EEPROM_CLI): $(CAN_DEVICE_LIB) $(SERIAL_CAN_LIB) $(CAN_EEPROM_LIB) FORCE
	make -C src/can_eeprom_cli

$(BMS_TUI): $(CAN_DEVICE_LIB) $(SERIAL_CAN_LIB) $(CAN_DATABASE_LIB) $(BMS_LIB) FORCE
	make -C src/bms_tui

sh: FORCE
	make -C src/sh

bat: FORCE
	make -C src/bat

plot: FORCE
	make -C src/plot

# Phony target, forces dependent targets to always be re-compiled
FORCE:

# Releasing
release: all FORCE
	# Create the release directory (delete if it already exists)
	if [ -d $(RELEASE_DIR) ]; then rm -Rf $(RELEASE_DIR); fi
	mkdir -p $(RELEASE_DIR)

	# Executables and configs
	cp -r $(BIN_DIR) $(RELEASE_DIR)/
	cp -r $(CONFIG_DIR) $(RELEASE_DIR)/

	# MSYS binaries (windows only)
	$(MSYS_UCRT_COPY_CMD)

	# Install / uninstall scripts
	cp $(INSTALLER) $(RELEASE_DIR)/
	chmod +x $(RELEASE_DIR)/$(INSTALLER)
	cp $(UNINSTALLER) $(RELEASE_DIR)/
	chmod +x $(RELEASE_DIR)/$(UNINSTALLER)

	# Documentation
	cp -r $(DOC_DIR) $(RELEASE_DIR)/
	cp $(RELEASE_README) $(RELEASE_DIR)/readme.txt

# Cleanup
clean:
	rm -r $(BIN_DIR)
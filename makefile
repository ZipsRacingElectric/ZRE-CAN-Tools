ROOT_DIR := .
include include.mk

# Releasing -------------------------------------------------------------------

# For some reason these can't be at the end of the file.

# Operating System Detection
# - This is only used for tagging releases, everything in here should work in
#   both linux and MSYS2.
ifeq ($(OS),Windows_NT)
	DETECTED_OS := windows
else
	DETECTED_OS := $(shell uname | tr '[:upper:]' '[:lower:]')
endif

ifeq ($(DETECTED_OS), windows)
	INSTALLER := install.bat
	UNINSTALLER := uninstall.bat
else
	INSTALLER := install.sh
	UNINSTALLER := uninstall.sh
endif

# Command for copying the MSYS2 UCRT binaries into a release
ifeq ($(DETECTED_OS), windows)
	ifeq ($(MSYS_BIN), "")
		MSYS_UCRT_COPY_CMD := echo "Error: MSYS_BIN environment variable is not declared!"" && exit -1
	else
		MSYS_UCRT_COPY_CMD := cp -r $(MSYS_BIN) $(RELEASE_DIR)/
	endif
endif

# Applications ----------------------------------------------------------------

# Default target. Compiles all applications in the src directory. Marked as
# phony because this corresponds to an actual directory name.
.PHONY: bin
bin: $(wildcard $(SRC_DIR)/*)

# Wildcard defining the compilation rule for each subdirectory in the src
# directory. Dependent on libs, meaning all libraries are compiled first.
$(SRC_DIR)/%: lib
	make -C $@

# Libraries -------------------------------------------------------------------

# Compilation rules for each library.

$(LIB_COMMON):
	make -C $(LIB_DIR)

# serial_can requires the -B option to actually compile binaries. Not sure why.
$(LIB_SERIAL_CAN):
	make -B -C $(LIB_DIR)/serial_can

$(LIB_CAN_DEVICE): $(LIB_SERIAL_CAN)
	make -C $(LIB_DIR)/can_device

$(LIB_CAN_DATABASE):
	make -C $(LIB_DIR)/can_database

$(LIB_CJSON):
	make -C $(LIB_DIR)/cjson

$(LIB_CAN_EEPROM):
	make -C $(LIB_DIR)/can_eeprom

$(LIB_BMS):
	make -C $(LIB_DIR)/bms

$(LIB_MDF):
	make -C $(LIB_DIR)/mdf

# Marks all library targets as phony. This forces the recipe to be re-run every
# time a dependent is ran. Without this, libraries would never be recompiled.
.PHONY: $(LIBS)

# Target for compiling all libraries. Marked as phony because this corresponds
# to an actual directory name.
.PHONY: lib
lib: $(LIBS)

# Make Targets ----------------------------------------------------------------

# Target for removing all compiled binaries.
.PHONY: clean
clean:
	rm -rf $(BIN_DIR)

# Releasing -------------------------------------------------------------------

# TODO(Barach): This should include the architecture, as we are now deploying on ARM.
RELEASE_DIR := $(ROOT_DIR)/release/zre_cantools_$(DETECTED_OS)_$(shell date +%Y.%m.%d)
RELEASE_README := doc/readme_release.txt

.PHONY: release
release: bin
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
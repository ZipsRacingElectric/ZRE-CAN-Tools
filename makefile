ROOT_DIR := .
SHELL := /bin/bash
include include.mk

# Releasing -------------------------------------------------------------------
# For some reason these can't be at the end of the file.

RELEASE_DIR := $(ROOT_DIR)/release/$(VERSION_FULL)
RELEASE_README := doc/readme_release.txt

ifeq ($(OS_TYPE), windows)
	INSTALLER := install.bat
	UNINSTALLER := uninstall.bat
else
	INSTALLER := install.sh
	UNINSTALLER := uninstall.sh
endif

# Command for copying the MSYS2 UCRT binaries into a release
ifeq ($(OS_TYPE),windows)
	ifeq ($(MSYS_BIN), "")
		MSYS_UCRT_COPY_CMD := echo "Error: MSYS_BIN environment variable is not declared!"" && exit -1
	else
		MSYS_UCRT_COPY_CMD := cp -r $(MSYS_BIN) $(RELEASE_DIR)/
	endif
endif

# VSCode Directory ------------------------------------------------------------

CLANGD_FILE := ./.vscode/compile_commands.json
VSCODE_SETTINGS := ./.vscode/settings.json
.PHONY: $(CLANGD_FILE)

# Applications ----------------------------------------------------------------

# Default target. Compiles all applications in the src directory. Marked as
# phony because this corresponds to an actual directory name.
bin: $(CLANGD_FILE) $(VSCODE_SETTINGS) $(wildcard $(SRC_DIR)/*)
.PHONY: bin

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

# VSCode Directory ------------------------------------------------------------

# Clangd-specific compilation flags. As there is only one clangd file for the
#   project, this should include any flag used in any application.
# Note: Not all warnings in gcc and clangd are one-to-one, so any extra to
#   enable / disable should be done here.
CLANGD_CFLAGS := $(CFLAGS) $(LIB_SERIAL_CAN_CFLAGS) $(LIB_GTK_CFLAGS)		\
	-Wno-newline-eof

# This awful script generates the Clangd compile_commands.json file. The
# horrible sed statement in the middle is for correctly escaping quotations, as
# the CFLAGS variable includes them. Specifically, Clangd expects escaped
# quotations in the form '\\\"'. The annoying part is printf expects both
# escaped quotes and backslashes, so really it must be given '\\\\\\\"'.
# It gets worse with the fact we are using printf twice, so said escaping must
# be repeated.

$(CLANGD_FILE):
	mkdir -p ./.vscode/
	rm -f $(CLANGD_FILE)
	printf "[\n"																				>> $(CLANGD_FILE)
	printf "\t{\n"																				>> $(CLANGD_FILE)
	printf "\t\t\"directory\": \"%q\",\n" "$(ABS_ROOT_DIR)"										>> $(CLANGD_FILE)
	printf "\t\t\"command\": \"gcc "															>> $(CLANGD_FILE)
	printf "%s,\n" "$(shell printf "%s" "$(CLANGD_CFLAGS)" | sed 's/"/\\\\\\\\\\\\\\\"/g')\""	>> $(CLANGD_FILE)
	printf "\t\t\"file\": \"*.c\"\n"															>> $(CLANGD_FILE)
	printf "\t}\n"																				>> $(CLANGD_FILE)
	printf "]\n"																				>> $(CLANGD_FILE)

# This generates the settings.json vscode file. Like above, this is pretty bad.
# Only thing that needs to be generated is the gcc binary, as that is different
# on Windows and Linux.
$(VSCODE_SETTINGS):
	mkdir -p ./.vscode/
	rm -f $(VSCODE_SETTINGS)
	printf "{\n"													>> $(VSCODE_SETTINGS)
	printf "\t\"C_Cpp.intelliSenseEngine\": \"disabled\",\n"		>> $(VSCODE_SETTINGS)
	printf "\t\"clangd.arguments\":\n"								>> $(VSCODE_SETTINGS)
	printf "\t[\n"													>> $(VSCODE_SETTINGS)
	printf "\t\t\"-log=verbose\",\n"								>> $(VSCODE_SETTINGS)
	printf "\t\t\"-pretty\",\n"										>> $(VSCODE_SETTINGS)
	printf "\t\t\"--background-index\",\n"							>> $(VSCODE_SETTINGS)
	printf "\t\t\"--compile-commands-dir=./.vscode/\",\n"			>> $(VSCODE_SETTINGS)
	printf "\t\t\"-header-insertion=never\",\n"						>> $(VSCODE_SETTINGS)
	printf "\t\t\"--query-driver=%q\"\n" "$(GCC_BIN)"				>> $(VSCODE_SETTINGS)
	printf "\t]\n"													>> $(VSCODE_SETTINGS)
	printf "}\n"													>> $(VSCODE_SETTINGS)

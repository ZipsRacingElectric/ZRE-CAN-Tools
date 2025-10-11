ROOT_DIR := ../..
include $(ROOT_DIR)/include.mk

ifeq ($(DETECTED_OS), windows)
	BAT := $(wildcard *.bat)
	BIN := $(patsubst %, $(BIN_DIR)/%, $(BAT))
else
	SH	:= $(wildcard *.sh)
	BIN	:= $(patsubst %.sh, $(BIN_DIR)/%, $(SH))
	EXT := .sh
endif

all: $(BIN)

$(BIN_DIR)/%: %$(EXT)
	mkdir -p $(BIN_DIR)
	cp $< $@
	chmod a+x $@
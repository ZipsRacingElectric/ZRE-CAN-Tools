ROOT_DIR := ../..
include $(ROOT_DIR)/include.mk

BAT := $(wildcard *.bat)
BIN := $(patsubst %, $(BIN_DIR)/%, $(BAT))

all: $(BIN)

$(BIN_DIR)/%: %
	mkdir -p $(BIN_DIR)
	cp $< $@
	chmod a+x $@
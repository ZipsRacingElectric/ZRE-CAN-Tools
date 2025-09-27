ROOT_DIR := ../..
include $(ROOT_DIR)/include.mk

SH	:= $(wildcard *.sh)
BIN	:= $(patsubst %.sh, $(BIN_DIR)/%, $(SH))

all: $(BIN)

$(BIN_DIR)/%: %.sh
	mkdir -p $(BIN_DIR)
	cp $< $@
	chmod a+x $@
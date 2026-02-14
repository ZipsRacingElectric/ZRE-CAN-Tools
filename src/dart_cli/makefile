ROOT_DIR := ../..
include $(ROOT_DIR)/include.mk

SH	:= $(wildcard *.sh)
BIN	:= $(patsubst %.sh, $(BIN_DIR)/%, $(SH))
EXT := .sh

all: $(BIN)

$(BIN_DIR)/%: %$(EXT)
	mkdir -p $(BIN_DIR)
	cp $< $@
	chmod a+x $@
	cp -r keys $(BIN_DIR)/
	chmod 600 $(BIN_DIR)/keys/id_rsa
	chmod 644 $(BIN_DIR)/keys/id_rsa.pub
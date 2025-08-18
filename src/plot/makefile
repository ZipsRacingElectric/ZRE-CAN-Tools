BIN_DIR := ../../bin

SH := $(patsubst %.sh, $(BIN_DIR)/%, $(wildcard *.sh))
M := $(patsubst %.m, $(BIN_DIR)/%.m, $(wildcard *.m))

all: $(SH) $(M)

$(BIN_DIR)/%: %.sh
	mkdir -p $(BIN_DIR)
	cp $< $@
	chmod a+x $@

$(BIN_DIR)/%.m: %.m
	mkdir -p $(BIN_DIR)
	cp $< $@
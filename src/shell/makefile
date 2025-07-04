BIN_DIR := ../../bin

SH := $(wildcard *.sh)
EXEC := $(patsubst %.sh, $(BIN_DIR)/%, $(SH))

all: $(EXEC)

$(BIN_DIR)/%: %.sh
	mkdir -p $(BIN_DIR)
	cp $< $@
	chmod a+x $@
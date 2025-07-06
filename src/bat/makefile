BIN_DIR := ../../bin

BAT := $(wildcard *.bat)
EXEC := $(patsubst %, $(BIN_DIR)/%, $(BAT))

all: $(EXEC)

$(BIN_DIR)/%: %
	mkdir -p $(BIN_DIR)
	cp $< $@
	chmod a+x $@
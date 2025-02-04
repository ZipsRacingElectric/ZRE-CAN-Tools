BIN_DIR	:= ../../bin
BIN		:= $(BIN_DIR)/can-dbc-tui

SRC 	:= main.c

$(BIN): $(SRC)
	mkdir -p $(BIN_DIR)
	gcc $(SRC) -lm -lncurses $(BIN_DIR)/can.a -I../../lib/ -o $(BIN)
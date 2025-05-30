CC_FLAGS := -Wall -Wextra -g

BIN_DIR	:= ../../bin
BIN		:= $(BIN_DIR)/bms-tui

SRC 	:=	main.c	\
			bms.c

$(BIN): $(SRC)
	mkdir -p $(BIN_DIR)
	gcc $(SRC) $(CC_FLAGS) -lncurses $(BIN_DIR)/can.a $(BIN_DIR)/cjson.a -I../../lib/ -o $(BIN) -lm
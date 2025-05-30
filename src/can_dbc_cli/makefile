CC_FLAGS := -Wall -Wextra -g

BIN_DIR	:= ../../bin
BIN		:= $(BIN_DIR)/can-dbc-cli

SRC 	:=	main.c

$(BIN): $(SRC)
	mkdir -p $(BIN_DIR)
	gcc $(SRC) $(CC_FLAGS) $(BIN_DIR)/can.a -I../../lib/ -o $(BIN) -lm
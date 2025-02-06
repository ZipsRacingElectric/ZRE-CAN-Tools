CC_FLAGS := -Wall -Wextra -g

BIN_DIR	:= ../../bin
BIN		:= $(BIN_DIR)/can-eeprom-cli

SRC 	:=	main.c					\
			can_eeprom.h			\
			can_eeprom.c			\
			can_eeprom_operations.h	\
			can_eeprom_operations.c

$(BIN): $(SRC)
	mkdir -p $(BIN_DIR)
	gcc $(SRC) $(CC_FLAGS) -lm $(BIN_DIR)/can.a $(BIN_DIR)/cjson.a -I../../lib/ -o $(BIN)
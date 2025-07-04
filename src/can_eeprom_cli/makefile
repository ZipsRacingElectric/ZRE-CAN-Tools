CC_FLAGS := -Wall -Wextra -g

BIN_DIR	:= ../../bin
LIB_DIR := $(BIN_DIR)/lib
BIN		:= $(BIN_DIR)/can-eeprom-cli

SRC 	:=	main.c

LIB		:=						\
	$(LIB_DIR)/can_device.a		\
	$(LIB_DIR)/can_eeprom.a		\
	$(LIB_DIR)/cjson.a			\
	$(LIB_DIR)/serial_can.a

$(BIN): $(SRC)
	mkdir -p $(BIN_DIR)
	gcc $(SRC) $(CC_FLAGS) $(LIB) -I../../lib/ -o $(BIN) -lm
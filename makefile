# Directories
BUILDDIR := bin

# Common source files
SRC :=	src/can_database.c	\
		src/can_dbc.c		\
		src/can_eeprom.c	\
		src/can_socket.c	\
		src/cjson_util.c

INC_FLAGS := -Isrc

# Libraries
include lib/cjson/cjson.mk

# Flags
LIB_FLAGS := -lm
CC_FLAGS := -Wall -Wextra $(INC_FLAGS) $(LIB_FLAGS)

# Targets
CAN_DBC_CLI := $(BUILDDIR)/can-dbc-cli
CAN_DBC_TUI := $(BUILDDIR)/can-dbc-tui
CAN_EEPROM_PROGRAMMER := $(BUILDDIR)/can-eeprom-programmer

all: $(CAN_DBC_CLI) $(CAN_DBC_TUI) $(CAN_EEPROM_PROGRAMMER)

$(CAN_DBC_CLI): src/can_dbc_cli/main.c $(SRC)
	mkdir -p $(BUILDDIR)
	gcc src/can_dbc_cli/main.c $(SRC) $(CC_FLAGS) -o $(CAN_DBC_CLI)

$(CAN_DBC_TUI): src/can_dbc_tui/main.c $(SRC)
	mkdir -p $(BUILDDIR)
	gcc src/can_dbc_tui/main.c $(SRC) $(CC_FLAGS) -lncurses -o $(CAN_DBC_TUI)

$(CAN_EEPROM_PROGRAMMER): src/can_eeprom_programmer/main.c $(SRC)
	mkdir -p $(BUILDDIR)
	gcc src/can_eeprom_programmer/main.c $(SRC) $(CC_FLAGS) -o $(CAN_EEPROM_PROGRAMMER)

clean:
	rm -r $(BUILDDIR)

vcan-create:
	ip link add dev vcan type vcan

vcan-set-up:
	ip link set up vcan
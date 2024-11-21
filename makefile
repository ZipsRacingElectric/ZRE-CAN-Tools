# Directories
BUILDDIR := bin

# Common source files
SRC :=	src/amk.h			\
		src/can_database.c	\
		src/can_dbc.c		\
		src/can_socket.c	\
		src/vcu.c

# Flags
LIB_FLAGS := -lm
CC_FLAGS := -Wall -Wextra $(LIB_FLAGS)

# Targets
CAN_DBC_CLI := $(BUILDDIR)/can-dbc-cli
CAN_DBC_TUI := $(BUILDDIR)/can-dbc-tui
VCU_CLI := $(BUILDDIR)/vcu-cli
AMK_CLI := $(BUILDDIR)/amk-cli

all: $(CAN_DBC_CLI) $(CAN_DBC_TUI) $(VCU_CLI) $(AMK_CLI)

$(CAN_DBC_CLI): src/can_dbc_cli.c $(SRC)
	mkdir -p $(BUILDDIR)
	gcc src/can_dbc_cli.c $(SRC) $(CC_FLAGS) -o $(CAN_DBC_CLI)

$(CAN_DBC_TUI): src/can_dbc_tui.c $(SRC)
	mkdir -p $(BUILDDIR)
	gcc src/can_dbc_tui.c $(SRC) $(CC_FLAGS) -lncurses -o $(CAN_DBC_TUI)

$(VCU_CLI): src/vcu_cli.c $(SRC)
	mkdir -p $(BUILDDIR)
	gcc src/vcu_cli.c $(SRC) $(CC_FLAGS) -o $(VCU_CLI)

$(AMK_CLI): src/amk_cli.c $(SRC)
	mkdir -p $(BUILDDIR)
	gcc src/amk_cli.c $(SRC) $(CC_FLAGS) -o $(AMK_CLI)

clean:
	rm -r $(BUILDDIR)

vcan-create:
	ip link add dev vcan type vcan

vcan-set-up:
	ip link set up vcan
BUILDDIR := bin

TARGET := $(BUILDDIR)/can-cli

SRC :=	src/main.c			\
		src/vcu.c			\
		src/can_database.c	\
		src/can_dbc.c		\
		src/can_socket.c

CC_FLAGS := -Wall -Wextra

LIB_FLAGS := -lm

$(TARGET): $(SRC)
	mkdir -p $(BUILDDIR)
	gcc $(SRC) $(CC_FLAGS) $(LIB_FLAGS) -o $(TARGET)

all: $(TARGET)

clean:
	rm -r $(BUILDDIR)

vcan-create:
	ip link add dev vcan type vcan

vcan-set-up:
	ip link set up vcan
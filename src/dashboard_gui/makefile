ROOT_DIR := ../..
include $(ROOT_DIR)/include.mk

BIN := $(BIN_DIR)/dashboard-gui
SRC :=	main.c										\
		gtk_util.c									\
		bms_widgets.c								\
		page_stack.c								\
		pages/page.c								\
		pages/page_drive.c							\
		pages/page_bms.c							\
		pages/page_can_bus.c						\
		can_widgets/can_widget.c					\
		can_widgets/can_indicator.c					\
		can_widgets/can_label_bool.c				\
		can_widgets/can_label_float.c				\
		can_widgets/can_progress_bar.c				\
		stylized_widgets/stylized_button.c			\
		stylized_widgets/stylized_frame.c			\
		stylized_widgets/stylized_progress_bar.c	\
		stylized_widgets/stylized_terminal.c

# Compiling and linking against GTK, so include its flags
CFLAGS += $(LIB_GTK_CFLAGS)
LIBFLAGS += $(LIB_GTK_LIBFLAGS)

# Note libraries must be in reverse order of dependencies, that is a dependency
# must be placed after its dependents.
LIB :=						\
	$(LIB_CJSON)			\
	$(LIB_BMS)				\
	$(LIB_CAN_DATABASE)		\
	$(LIB_CAN_DEVICE)		\
	$(LIB_SERIAL_CAN)		\
	$(LIB_COMMON)

all: $(BIN)

$(BIN): $(SRC) $(LIB)
	mkdir -p $(BIN_DIR)
	gcc $^ -fno-inline $(CFLAGS) -o $@ $(LIBFLAGS)

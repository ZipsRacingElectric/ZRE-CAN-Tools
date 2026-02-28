ROOT_DIR := ../..
include $(ROOT_DIR)/include.mk

BIN := $(BIN_DIR)/dashboard-gui
SRC :=	main.c										\
		gtk_util.c									\
		page_stack.c								\
		pages/page.c								\
		pages/page_drive.c							\
		pages/page_bms.c							\
		pages/page_can_bus.c						\
		pages/page_status.c							\
		pages/page_template.c						\
		can_widgets/can_widget.c					\
		can_widgets/can_indicator.c					\
		can_widgets/can_label_bool.c				\
		can_widgets/can_label_float.c				\
		can_widgets/can_label_fault.c				\
		can_widgets/can_progress_bar.c				\
		can_widgets/can_shutdown_loop_indicator.c	\
		can_widgets/can_fault_popup.c				\
		can_widgets/can_float_popup.c				\
		can_widgets/can_widget_template.c			\
		stylized_widgets/stylized_bar_graph.c		\
		stylized_widgets/stylized_button.c			\
		stylized_widgets/stylized_frame.c			\
		stylized_widgets/stylized_progress_bar.c	\
		stylized_widgets/stylized_terminal.c		\
		bms_widgets/cell_voltage_bar_graph.c		\
		bms_widgets/sense_line_temperature_graph.c

# Compiling and linking against GTK, so include its flags
CFLAGS += $(LIB_GTK_CFLAGS)
LIBFLAGS += $(LIB_GTK_LIBFLAGS)

# Note libraries must be in reverse order of dependencies, that is a dependency
# must be placed after its dependents.
LIB :=						\
	$(LIB_CJSON)			\
	$(LIB_BMS)				\
	$(LIB_CAN_NODE)			\
	$(LIB_CAN_DATABASE)		\
	$(LIB_CAN_DEVICE)		\
	$(LIB_SERIAL_CAN)		\
	$(LIB_COMMON)

all: $(BIN)

$(BIN): $(SRC) $(LIB)
	mkdir -p $(BIN_DIR)
	gcc $^ -fno-inline $(CFLAGS) -o $@ $(LIBFLAGS)

// Header
#include "page_autox.h"

// Includes
#include "gtk_util.h"
#include "can_widgets/can_label_bool.h"
#include "can_widgets/can_label_float.h"
#include "can_widgets/can_progress_bar.h"
#include "can_widgets/can_indicator.h"
#include "stylized_button.h"

#define BUTTON_LABEL_FONT		"Futura Std Bold Condensed 34px"
#define DATA_LOGGER_TITLE_FONT	"Futura Std Bold Condensed 36px"
#define DATA_LOGGER_STAT_FONT	"ITC Avant Garde Gothic CE Book 22px"
#define CENTER_TITLE_FONT		"ITC Avant Garde Gothic CE Book 22px"
#define CENTER_VALUE_FONT		"Technology Bold 200px"
#define PANEL_TITLE_FONT		"Futura Std Bold Condensed 26px"
#define PANEL_STAT_FONT			"ITC Avant Garde Gothic CE Book 26px"
#define FAULT_NAME_FONT			"Futura Std Bold Condensed 32px @color=#000000"

const canIndicatorPoint_t FAULT_INDICATOR_POLYGON [] =
{
	{0.50, 0.00},
	{0.09, 0.00},
	{0.09, 0.02},
	{0.18, 0.40},
	{0.18, 0.67},
	{0.01, 0.02},
	{0.09, 0.02},
	{0.09, 0.00},
	{0.00, 0.00},
	{0.00, 0.50},
	{0.01, 0.45},
	{0.15, 0.98},
	{0.06, 0.98},
	{0.01, 0.80},
	{0.01, 0.45},
	{0.00, 0.45},
	{0.00, 1.00},
	{0.50, 1.00},
	{0.50, 0.98},
	{0.18, 0.98},
	{0.18, 0.02},
	{0.50, 0.02},
	{0.50, 0.06},
	{0.50, 0.94},
	{0.20, 0.94},
	{0.20, 0.06},
	{0.50, 0.06},

	{0.50, 0.06},
	{0.80, 0.06},
	{0.80, 0.94},
	{0.50, 0.94},
	{0.50, 0.06},
	{0.50, 0.02},
	{0.82, 0.02},
	{0.82, 0.98},
	{0.50, 0.98},
	{0.50, 1.00},
	{1.00, 1.00},
	{1.00, 0.45},
	{0.99, 0.45},
	{0.99, 0.80},
	{0.94, 0.98},
	{0.85, 0.98},
	{0.99, 0.45},
	{1.00, 0.50},
	{1.00, 0.00},
	{0.91, 0.00},
	{0.91, 0.02},
	{0.99, 0.02},
	{0.82, 0.67},
	{0.82, 0.40},
	{0.91, 0.02},
	{0.91, 0.00},
	{0.50, 0.00},
};
const size_t FAULT_INDICATOR_POLYGON_SIZE = sizeof (FAULT_INDICATOR_POLYGON) / sizeof (canIndicatorPoint_t);

static void drawBg (GtkDrawingArea* area, cairo_t* cr, int width, int height, gpointer arg)
{
	pageAutox_t* page = arg;
	(void) area;
	(void) arg;

	GdkRGBA c = gdkHexToColor ("#000000");
	cairo_rectangle (cr, 0, 0, width, height);
	cairo_fill (cr);

	graphene_rect_t bounds;
	if (!gtk_widget_compute_bounds (GTK_WIDGET (page->grid), GTK_WIDGET (area), &bounds))
		bounds = *graphene_rect_zero ();

	float xGMin = 0;
	float yGMin = bounds.origin.y;
	float xGMax = width;
	float yGMax = bounds.origin.y + bounds.size.height;

	c = gdkHexToColor ("#FF00FF");
	gdk_cairo_set_source_rgba (cr, &c);

	if (!gtk_widget_compute_bounds (CAN_WIDGET_TO_WIDGET (page->apps), GTK_WIDGET (area), &bounds))
		bounds = *graphene_rect_zero ();

	xGMax -= bounds.size.width;

	cairo_pattern_t* gradient;
	gradient = cairo_pattern_create_linear (xGMin, yGMin, xGMax, yGMin);
	cairo_pattern_add_color_stop_rgb (gradient, 0, 0.714, 0.239, 0.141);
	cairo_pattern_add_color_stop_rgb (gradient, 0.4, 0.780, 0.580, 0.094);
	cairo_pattern_add_color_stop_rgb (gradient, 1, 0.051, 0.369, 0.173);

	cairo_set_source (cr, gradient);

	cairo_rectangle (cr, xGMin, yGMin, xGMax - xGMin, yGMax - yGMin);
	cairo_fill (cr);
	cairo_pattern_destroy (gradient);

	if (!gtk_widget_compute_bounds (GTK_WIDGET (page->dataLoggerPanel), GTK_WIDGET (area), &bounds))
		bounds = *graphene_rect_zero ();

	float x0 = bounds.origin.x;
	float y0 = bounds.origin.y;

	float x1 = bounds.origin.x + bounds.size.width;
	float y1 = bounds.origin.y;

	if (!gtk_widget_compute_bounds (GTK_WIDGET (page->centerLabel), GTK_WIDGET (area), &bounds))
		bounds = *graphene_rect_zero ();

	float x2 = bounds.origin.x;
	float y2 = bounds.origin.y;

	x1 -= (y2 - y1) / 2;

	if (!gtk_widget_compute_bounds (GTK_WIDGET (page->rightPanel), GTK_WIDGET (area), &bounds))
		bounds = *graphene_rect_zero ();

	float x3 = bounds.origin.x + bounds.size.width;
	float y3 = y2;

	if (!gtk_widget_compute_bounds (CAN_WIDGET_TO_WIDGET (page->speed), GTK_WIDGET (area), &bounds))
		bounds = *graphene_rect_zero ();

	float x4 = x3;
	float y4 = bounds.origin.y + bounds.size.height;

	if (!gtk_widget_compute_bounds (GTK_WIDGET (page->leftPanel), GTK_WIDGET (area), &bounds))
		bounds = *graphene_rect_zero ();

	float x5 = bounds.origin.x;
	float y5 = y4;

	if (!gtk_widget_compute_bounds (CAN_WIDGET_TO_WIDGET (page->bse), GTK_WIDGET (area), &bounds))
		bounds = *graphene_rect_zero ();

	float x6 = bounds.origin.x + bounds.size.width + 30;
	float y6 = yGMax;

	float x7 = x6 - 20;
	float y7 = y6;

	if (!gtk_widget_compute_bounds (GTK_WIDGET (page->dataLoggerPanel), GTK_WIDGET (area), &bounds))
		bounds = *graphene_rect_zero ();

	float x8 = x5 - 30;
	float y8 = bounds.origin.y + bounds.size.height;

	float x9 = bounds.origin.x;
	float y9 = y8;

	float x10 = x5;
	float y10 = y8;

	float y19 = y8;
	float x19 = bounds.origin.x + bounds.size.width - (y19 - y2) / 2;

	if (!gtk_widget_compute_bounds (GTK_WIDGET (page->faultPanel), GTK_WIDGET (area), &bounds))
		bounds = *graphene_rect_zero ();

	float x11 = bounds.origin.x;
	float y11 = bounds.origin.y;

	float x12 = x11 + bounds.size.width;
	float y12 = y11;

	float x13 = x12;
	float y13 = y12 + bounds.size.height;

	float x14 = x11 - (y13 - y11) / 2;
	float y14 = y13;

	if (!gtk_widget_compute_bounds (GTK_WIDGET (page->buttonPanel), GTK_WIDGET (area), &bounds))
		bounds = *graphene_rect_zero ();

	float x15 = bounds.origin.x;
	float y15 = bounds.origin.y;

	float x16 = x15 + bounds.size.width;
	float y16 = y15;

	float x17 = x16;
	float y17 = y16 + bounds.size.height;

	float x18 = x6 + 10;
	float y18 = y17;

	c = gdkHexToColor ("#000000");
	gdk_cairo_set_source_rgba (cr, &c);
	cairo_move_to (cr, x0, y0);
	cairo_line_to (cr, x1, y1);
	cairo_line_to (cr, x2, y2);
	cairo_line_to (cr, x3, y3);
	cairo_line_to (cr, x4, y4);
	cairo_line_to (cr, x5, y5);
	cairo_line_to (cr, x6, y6);
	cairo_line_to (cr, x7, y7);
	cairo_line_to (cr, x8, y8);
	cairo_line_to (cr, x9, y9);
	cairo_fill (cr);

	cairo_move_to (cr, x11, y11);
	cairo_line_to (cr, x12, y12);
	cairo_line_to (cr, x13, y13);
	cairo_line_to (cr, x14, y14);
	cairo_fill (cr);

	cairo_move_to (cr, x15, y15);
	cairo_line_to (cr, x16, y16);
	cairo_line_to (cr, x17, y17);
	cairo_line_to (cr, x18, y18);
	cairo_fill (cr);

	c = gdkHexToColor ("#D3792C");
	gdk_cairo_set_source_rgba (cr, &c);
	cairo_set_line_width (cr, 1);

	cairo_move_to (cr, x10, y10);
	cairo_line_to (cr,  x9,  y9);
	cairo_line_to (cr,  x0,  y0);
	cairo_line_to (cr,  x1,  y1);
	cairo_line_to (cr,  x2,  y2);
	cairo_line_to (cr,  x3,  y3);
	cairo_line_to (cr,  x4,  y4);
	cairo_line_to (cr,  x5,  y5);
	cairo_line_to (cr, x10, y10);
	cairo_line_to (cr, x19, y19);
	cairo_line_to (cr,  x2,  y2);
	cairo_stroke (cr);

	cairo_move_to (cr, x11, y11);
	cairo_line_to (cr, x12, y12);
	cairo_line_to (cr, x13, y13);
	cairo_line_to (cr, x14, y14);
	cairo_line_to (cr, x11, y11);
	cairo_stroke (cr);

	if (!gtk_widget_compute_bounds (GTK_WIDGET (page->leftPanel), GTK_WIDGET (area), &bounds))
		bounds = *graphene_rect_zero ();
	cairo_rectangle (cr, bounds.origin.x, bounds.origin.y, bounds.size.width, bounds.size.height);
	cairo_stroke (cr);

	if (!gtk_widget_compute_bounds (GTK_WIDGET (page->rightPanel), GTK_WIDGET (area), &bounds))
		bounds = *graphene_rect_zero ();
	cairo_rectangle (cr, bounds.origin.x, bounds.origin.y, bounds.size.width, bounds.size.height);
	cairo_stroke (cr);
}

page_t* pageAutoxInit (canDatabase_t* database)
{
	const GdkRGBA FONT_COLOR = gdkHexToColor ("#F4931E");

	// Allocate the page
	pageAutox_t* page = malloc (sizeof (pageAutox_t));
	if (page == NULL)
		return NULL;

	// Setup the VMT
	page->vmt.update = pageAutoxUpdate;
	page->vmt.appendButton = pageAutoxAppendButton;
	page->vmt.widget = gtk_overlay_new ();

	page->buttonCount = 0;

	GtkWidget* bg = gtk_drawing_area_new ();
	gtk_drawing_area_set_draw_func (GTK_DRAWING_AREA (bg), drawBg, page, NULL);
	gtk_overlay_set_child (GTK_OVERLAY (page->vmt.widget), bg);

	page->grid = GTK_GRID (gtk_grid_new ());
	gtk_widget_set_margin_top (GTK_WIDGET (page->grid), 10);
	gtk_widget_set_margin_bottom (GTK_WIDGET (page->grid), 10);
	gtk_overlay_add_overlay (GTK_OVERLAY (page->vmt.widget), GTK_WIDGET (page->grid));

	// BSE bar
	page->bse = canProgressBarInit (database, &(canProgressBarConfig_t)
	{
		.signalName = "BSE_FRONT_PERCENT",
		.min		= 0,
		.max		= 100
	});
	gtk_orientable_set_orientation (CAN_PROGRESS_BAR_TO_ORIENTABLE (page->bse), GTK_ORIENTATION_VERTICAL);
	gtk_progress_bar_set_inverted (CAN_PROGRESS_BAR_TO_PROGRESS_BAR (page->bse), true);
	gtk_widget_set_vexpand (CAN_WIDGET_TO_WIDGET (page->bse), true);
	gtk_widget_set_size_request (CAN_WIDGET_TO_WIDGET (page->bse), 20, 0);
	gtk_grid_attach (page->grid, CAN_WIDGET_TO_WIDGET (page->bse), 0, 0, 1, 5);

	page->dataLoggerPanel = GTK_GRID (gtk_grid_new ());
	gtk_grid_attach (page->grid, GTK_WIDGET (page->dataLoggerPanel), 1, 0, 2, 2);
	gtk_widget_set_valign (GTK_WIDGET (page->dataLoggerPanel), GTK_ALIGN_END);

	page->dataLoggerStatus = canLabelBoolInit (database, &(canLabelBoolConfig_t)
	{
		.signalName		= "SESSION_NUMBER",
		.activeValue	= "Logging: On",
		.inactiveValue	= "Logging: On",
		.invalidValue	= "Logging: Off",
		.threshold		= 0,
		.inverted		= true
	});
	gtkLabelSetFont (CAN_LABEL_BOOL_TO_LABEL (page->dataLoggerStatus), DATA_LOGGER_TITLE_FONT);
	gtkLabelSetColor (CAN_LABEL_BOOL_TO_LABEL (page->dataLoggerStatus), &FONT_COLOR);
	gtk_label_set_xalign (CAN_LABEL_BOOL_TO_LABEL (page->dataLoggerStatus), 0);
	gtk_widget_set_margin_top (CAN_WIDGET_TO_WIDGET (page->dataLoggerStatus), 3);
	gtk_widget_set_margin_start (CAN_WIDGET_TO_WIDGET (page->dataLoggerStatus), 3);
	gtk_grid_attach (page->dataLoggerPanel, CAN_WIDGET_TO_WIDGET (page->dataLoggerStatus), 0, 0, 1, 1);

	page->dataLoggerSession = canLabelFloatInit (database, &(canLabelFloatConfig_t)
	{
		.signalName		= "SESSION_NUMBER",
		.formatValue	= "Session\nNo. %.0f",
		.formatInvalid	= "Session\nNo. %s",
	});
	gtkLabelSetFont (CAN_LABEL_FLOAT_TO_LABEL (page->dataLoggerSession), DATA_LOGGER_STAT_FONT);
	gtkLabelSetColor (CAN_LABEL_BOOL_TO_LABEL (page->dataLoggerSession), &FONT_COLOR);
	gtk_label_set_xalign (CAN_LABEL_FLOAT_TO_LABEL (page->dataLoggerSession), 0);
	gtk_widget_set_margin_start (CAN_WIDGET_TO_WIDGET (page->dataLoggerSession), 3);
	gtk_grid_attach (page->dataLoggerPanel, CAN_WIDGET_TO_WIDGET (page->dataLoggerSession), 0, 1, 1, 1);

	GtkWidget* padding = gtk_grid_new ();
	gtk_widget_set_size_request (padding, 80, 0);
	gtk_grid_attach (page->grid, padding, 1, 2, 1, 1);

	page->leftPanel = GTK_GRID (gtk_grid_new ());
	gtk_widget_set_margin_end (GTK_WIDGET (page->leftPanel), 10);
	gtk_widget_set_valign (GTK_WIDGET (page->leftPanel), GTK_ALIGN_CENTER);
	gtk_grid_attach (page->grid, GTK_WIDGET (page->leftPanel), 2, 2, 1, 1);

	GtkWidget* label = gtk_label_new ("Torque Config");
	gtkLabelSetFont (GTK_LABEL (label), PANEL_TITLE_FONT);
	gtkLabelSetColor (GTK_LABEL (label), &FONT_COLOR);
	gtk_widget_set_size_request (label, 180, 0);
	gtk_label_set_xalign (GTK_LABEL (label), 0.5);
	gtk_widget_set_margin_top (label, 3);
	gtk_grid_attach (page->leftPanel, label, 0, 0, 2, 1);

	label = gtk_label_new ("Driving:");
	gtkLabelSetFont (GTK_LABEL (label), PANEL_STAT_FONT);
	gtkLabelSetColor (GTK_LABEL (label), &FONT_COLOR);
	gtk_widget_set_halign (label, GTK_ALIGN_START);
	gtk_widget_set_margin_start (label, 3);
	gtk_grid_attach (page->leftPanel, label, 0, 1, 1, 1);

	page->drivingTorque = canLabelFloatInit (database, &(canLabelFloatConfig_t)
	{
		.signalName		= "DRIVING_TORQUE_LIMIT",
		.formatValue	= "%.0f%s",
		.formatInvalid	= "%s%s"
	});
	gtkLabelSetFont (CAN_LABEL_FLOAT_TO_LABEL (page->drivingTorque), PANEL_STAT_FONT);
	gtkLabelSetColor (CAN_LABEL_BOOL_TO_LABEL (page->drivingTorque), &FONT_COLOR);
	gtk_widget_set_halign (CAN_WIDGET_TO_WIDGET (page->drivingTorque), GTK_ALIGN_END);
	gtk_widget_set_margin_end (CAN_WIDGET_TO_WIDGET (page->drivingTorque), 3);
	gtk_grid_attach (page->leftPanel, CAN_WIDGET_TO_WIDGET (page->drivingTorque), 1, 1, 1, 1);

	label = gtk_label_new ("Regen:");
	gtkLabelSetFont (GTK_LABEL (label), PANEL_STAT_FONT);
	gtkLabelSetColor (GTK_LABEL (label), &FONT_COLOR);
	gtk_widget_set_halign (label, GTK_ALIGN_START);
	gtk_widget_set_margin_start (label, 3);
	gtk_grid_attach (page->leftPanel, label, 0, 2, 1, 1);

	page->regenTorque = canLabelFloatInit (database, &(canLabelFloatConfig_t)
	{
		.signalName		= "REGEN_TORQUE_LIMIT",
		.formatValue	= "%.0f%s",
		.formatInvalid	= "%s%s"
	});
	gtkLabelSetFont (CAN_LABEL_FLOAT_TO_LABEL (page->regenTorque), PANEL_STAT_FONT);
	gtkLabelSetColor (CAN_LABEL_BOOL_TO_LABEL (page->regenTorque), &FONT_COLOR);
	gtk_widget_set_halign (CAN_WIDGET_TO_WIDGET (page->regenTorque), GTK_ALIGN_END);
	gtk_widget_set_margin_end (CAN_WIDGET_TO_WIDGET (page->regenTorque), 3);
	gtk_grid_attach (page->leftPanel, CAN_WIDGET_TO_WIDGET (page->regenTorque), 1, 2, 1, 1);

	label = gtk_label_new ("Index:");
	gtkLabelSetFont (GTK_LABEL (label), PANEL_STAT_FONT);
	gtkLabelSetColor (GTK_LABEL (label), &FONT_COLOR);
	gtk_widget_set_halign (label, GTK_ALIGN_START);
	gtk_widget_set_margin_start (label, 3);
	gtk_grid_attach (page->leftPanel, label, 0, 3, 1, 1);

	page->torqueIndex = canLabelFloatInit (database, &(canLabelFloatConfig_t)
	{
		.signalName		= "TORQUE_ALGORITHM_INDEX",
		.formatValue	= "%.0f",
		.formatInvalid	= "%s"
	});
	gtkLabelSetFont (CAN_LABEL_FLOAT_TO_LABEL (page->torqueIndex), PANEL_STAT_FONT);
	gtkLabelSetColor (CAN_LABEL_BOOL_TO_LABEL (page->torqueIndex), &FONT_COLOR);
	gtk_widget_set_halign (CAN_WIDGET_TO_WIDGET (page->torqueIndex), GTK_ALIGN_END);
	gtk_widget_set_margin_end (CAN_WIDGET_TO_WIDGET (page->torqueIndex), 3);
	gtk_grid_attach (page->leftPanel, CAN_WIDGET_TO_WIDGET (page->torqueIndex), 1, 3, 1, 1);

	label = gtk_label_new ("DRS:");
	gtkLabelSetFont (GTK_LABEL (label), PANEL_STAT_FONT);
	gtkLabelSetColor (GTK_LABEL (label), &FONT_COLOR);
	gtk_widget_set_halign (label, GTK_ALIGN_START);
	gtk_widget_set_margin_start (label, 3);
	gtk_grid_attach (page->leftPanel, label, 0, 4, 1, 1);

	page->drsStatus = canIndicatorInit (database, &(canIndicatorConfig_t)
	{
		.signalName 	= "TODO",
		.threshold		= 0.5f,
		.inverted		= false,
		.width			= 26,
		.height			= 26,
		.activeColor	= gdkHexToColor ("#FFFFFF"),
		.inactiveColor	= gdkHexToColor ("#580000"),
		.invalidColor	= gdkHexToColor ("#FF0000"),
		.polygon		= NULL,
		.polygonSize	= 0
	});
	gtk_widget_set_halign (CAN_WIDGET_TO_WIDGET (page->drsStatus), GTK_ALIGN_END);
	gtk_widget_set_margin_end (CAN_WIDGET_TO_WIDGET (page->drsStatus), 3);
	gtk_grid_attach (page->leftPanel, CAN_WIDGET_TO_WIDGET (page->drsStatus), 1, 4, 1, 1);

	page->faultPanel = GTK_GRID (gtk_grid_new ());
	gtk_widget_set_margin_top (GTK_WIDGET (page->faultPanel), 20);
	gtk_widget_set_margin_bottom (GTK_WIDGET (page->faultPanel), 20);
	gtk_widget_set_margin_start (GTK_WIDGET (page->faultPanel), 40);
	gtk_grid_attach (page->grid, GTK_WIDGET (page->faultPanel), 3, 0, 2, 1);

	page->vcuFault = canIndicatorInit (database, &(canIndicatorConfig_t)
	{
		.signalName		= "VCU_FAULT",
		.threshold		= 0.5f,
		.inverted		= false,
		.width			= 100,
		.height			= 42,
		.activeColor	= gdkHexToColor ("#FF0000"),
		.inactiveColor	= gdkHexToColor ("#580000"),
		.invalidColor	= gdkHexToColor ("#FF0000"),
		.polygon		= FAULT_INDICATOR_POLYGON,
		.polygonSize	= FAULT_INDICATOR_POLYGON_SIZE,
	});
	gtk_widget_set_hexpand (CAN_WIDGET_TO_WIDGET (page->vcuFault), true);
	gtk_widget_set_margin_top (CAN_WIDGET_TO_WIDGET (page->vcuFault), 5);
	gtk_widget_set_margin_bottom (CAN_WIDGET_TO_WIDGET (page->vcuFault), 5);
	gtk_grid_attach (page->faultPanel, CAN_WIDGET_TO_WIDGET (page->vcuFault), 0, 0, 1, 1);

	label = gtk_label_new ("VCU");
	gtkLabelSetFont (GTK_LABEL (label), FAULT_NAME_FONT);
	const GdkRGBA COLOR_BLACK = gdkHexToColor ("#000000");
	gtkLabelSetColor (GTK_LABEL (label), &COLOR_BLACK);
	gtk_grid_attach (page->faultPanel, label, 0, 0, 1, 1);

	page->bmsFault = canIndicatorInit (database, &(canIndicatorConfig_t)
	{
		.signalName		= "BMS_FAULT",
		.threshold		= 0.5f,
		.inverted		= false,
		.width			= 100,
		.height			= 42,
		.activeColor	= gdkHexToColor ("#FF0000"),
		.inactiveColor	= gdkHexToColor ("#580000"),
		.invalidColor	= gdkHexToColor ("#FF0000"),
		.polygon		= FAULT_INDICATOR_POLYGON,
		.polygonSize	= FAULT_INDICATOR_POLYGON_SIZE,
	});
	gtk_widget_set_hexpand (CAN_WIDGET_TO_WIDGET (page->bmsFault), true);
	gtk_grid_attach (page->faultPanel, CAN_WIDGET_TO_WIDGET (page->bmsFault), 1, 0, 1, 1);

	label = gtk_label_new ("BMS");
	gtkLabelSetFont (GTK_LABEL (label), FAULT_NAME_FONT);
	gtkLabelSetColor (GTK_LABEL (label), &COLOR_BLACK);
	gtk_grid_attach (page->faultPanel, label, 1, 0, 1, 1);

	page->amkFault = canIndicatorInit (database, &(canIndicatorConfig_t)
	{
		.signalName		= "AMK_FAULT",
		.threshold		= 0.5f,
		.inverted		= false,
		.width			= 100,
		.height			= 42,
		.activeColor	= gdkHexToColor ("#FF0000"),
		.inactiveColor	= gdkHexToColor ("#580000"),
		.invalidColor	= gdkHexToColor ("#FF0000"),
		.polygon		= FAULT_INDICATOR_POLYGON,
		.polygonSize	= FAULT_INDICATOR_POLYGON_SIZE,
	});
	gtk_widget_set_hexpand (CAN_WIDGET_TO_WIDGET (page->amkFault), true);
	gtk_grid_attach (page->faultPanel, CAN_WIDGET_TO_WIDGET (page->amkFault), 2, 0, 1, 1);

	label = gtk_label_new ("AMK");
	gtkLabelSetFont (GTK_LABEL (label), FAULT_NAME_FONT);
	gtkLabelSetColor (GTK_LABEL (label), &COLOR_BLACK);
	gtk_grid_attach (page->faultPanel, label, 2, 0, 1, 1);

	page->gpsFault = canIndicatorInit (database, &(canIndicatorConfig_t)
	{
		.signalName		= "GPS_STATUS",
		.threshold		= 2.5f,
		.inverted		= true,
		.width			= 100,
		.height			= 42,
		.activeColor	= gdkHexToColor ("#FF0000"),
		.inactiveColor	= gdkHexToColor ("#404040"),
		.invalidColor	= gdkHexToColor ("#FF0000"),
		.polygon		= FAULT_INDICATOR_POLYGON,
		.polygonSize	= FAULT_INDICATOR_POLYGON_SIZE,
	});
	gtk_widget_set_hexpand (CAN_WIDGET_TO_WIDGET (page->gpsFault), true);
	gtk_grid_attach (page->faultPanel, CAN_WIDGET_TO_WIDGET (page->gpsFault), 3, 0, 1, 1);

	label = gtk_label_new ("GPS");
	gtkLabelSetFont (GTK_LABEL (label), FAULT_NAME_FONT);
	gtkLabelSetColor (GTK_LABEL (label), &COLOR_BLACK);
	gtk_grid_attach (page->faultPanel, label, 3, 0, 1, 1);

	page->centerLabel = gtk_label_new ("Vehicle Speed (Km/h):");
	gtkLabelSetFont (GTK_LABEL (page->centerLabel), CENTER_TITLE_FONT);
	gtkLabelSetColor (GTK_LABEL (page->centerLabel), &FONT_COLOR);
	gtk_label_set_xalign (GTK_LABEL (page->centerLabel), 0);
	gtk_grid_attach (page->grid, page->centerLabel, 3, 1, 1, 1);

	page->speed = canLabelFloatInit (database, &(canLabelFloatConfig_t)
	{
		.signalName		= "SPEED",
		.formatValue	= "%02.0f",
		.formatInvalid	= "XX"
	});
	gtkLabelSetFont (CAN_LABEL_FLOAT_TO_LABEL (page->speed), CENTER_VALUE_FONT);
	gtkLabelSetColor (CAN_LABEL_BOOL_TO_LABEL (page->speed), &FONT_COLOR);
	gtk_widget_set_hexpand (CAN_WIDGET_TO_WIDGET (page->speed), true);
	gtk_widget_set_vexpand (CAN_WIDGET_TO_WIDGET (page->speed), true);
	gtk_grid_attach (page->grid, CAN_WIDGET_TO_WIDGET (page->speed), 3, 2, 1, 1);

	page->rightPanel = GTK_GRID (gtk_grid_new ());
	gtk_widget_set_margin_start (GTK_WIDGET (page->rightPanel), 10);
	gtk_widget_set_valign (GTK_WIDGET (page->rightPanel), GTK_ALIGN_CENTER);
	gtk_grid_attach (page->grid, GTK_WIDGET (page->rightPanel), 4, 2, 1, 1);

	label = gtk_label_new ("Powertrain");
	gtkLabelSetFont (GTK_LABEL (label), PANEL_TITLE_FONT);
	gtkLabelSetColor (GTK_LABEL (label), &FONT_COLOR);
	gtk_widget_set_size_request (label, 180, 0);
	gtk_label_set_xalign (GTK_LABEL (label), 0.5);
	gtk_widget_set_margin_top (label, 3);
	gtk_grid_attach (page->rightPanel, label, 0, 0, 2, 1);

	label = gtk_label_new ("GLV:");
	gtkLabelSetFont (GTK_LABEL (label), PANEL_STAT_FONT);
	gtkLabelSetColor (GTK_LABEL (label), &FONT_COLOR);
	gtk_widget_set_halign (label, GTK_ALIGN_START);
	gtk_widget_set_margin_start (label, 3);
	gtk_grid_attach (page->rightPanel, label, 0, 1, 1, 1);

	page->glvVoltage = canLabelFloatInit (database, &(canLabelFloatConfig_t)
	{
		.signalName		= "GLV_BATTERY_VOLTAGE",
		.formatValue	= "%.1f %s",
		.formatInvalid	= "%s %s"
	});
	gtkLabelSetFont (CAN_LABEL_FLOAT_TO_LABEL (page->glvVoltage), PANEL_STAT_FONT);
	gtkLabelSetColor (CAN_LABEL_BOOL_TO_LABEL (page->glvVoltage), &FONT_COLOR);
	gtk_widget_set_halign (CAN_WIDGET_TO_WIDGET (page->glvVoltage), GTK_ALIGN_END);
	gtk_widget_set_margin_end (CAN_WIDGET_TO_WIDGET (page->glvVoltage), 3);
	gtk_grid_attach (page->rightPanel, CAN_WIDGET_TO_WIDGET (page->glvVoltage), 1, 1, 1, 1);

	label = gtk_label_new ("HV:");
	gtkLabelSetFont (GTK_LABEL (label), PANEL_STAT_FONT);
	gtkLabelSetColor (GTK_LABEL (label), &FONT_COLOR);
	gtk_widget_set_halign (label, GTK_ALIGN_START);
	gtk_widget_set_margin_start (label, 3);
	gtk_grid_attach (page->rightPanel, label, 0, 2, 1, 1);

	page->hvVoltage = canLabelFloatInit (database, &(canLabelFloatConfig_t)
	{
		.signalName		= "PACK_VOLTAGE",
		.formatValue	= "%.1f %s",
		.formatInvalid	= "%s %s"
	});
	gtkLabelSetFont (CAN_LABEL_FLOAT_TO_LABEL (page->hvVoltage), PANEL_STAT_FONT);
	gtkLabelSetColor (CAN_LABEL_BOOL_TO_LABEL (page->hvVoltage), &FONT_COLOR);
	gtk_widget_set_halign (CAN_WIDGET_TO_WIDGET (page->hvVoltage), GTK_ALIGN_END);
	gtk_widget_set_margin_end (CAN_WIDGET_TO_WIDGET (page->hvVoltage), 3);
	gtk_grid_attach (page->rightPanel, CAN_WIDGET_TO_WIDGET (page->hvVoltage), 1, 2, 1, 1);

	label = gtk_label_new ("Inv:");
	gtkLabelSetFont (GTK_LABEL (label), PANEL_STAT_FONT);
	gtkLabelSetColor (GTK_LABEL (label), &FONT_COLOR);
	gtk_widget_set_halign (label, GTK_ALIGN_START);
	gtk_widget_set_margin_start (label, 3);
	gtk_grid_attach (page->rightPanel, label, 0, 3, 1, 1);

	page->inverterMaxTemp = canLabelFloatInit (database, &(canLabelFloatConfig_t)
	{
		.signalName		= "AMK_INVERTER_TEMPERATURE_MAX",
		.formatValue	= "%.1f %s",
		.formatInvalid	= "%s %s"
	});
	gtkLabelSetFont (CAN_LABEL_FLOAT_TO_LABEL (page->inverterMaxTemp), PANEL_STAT_FONT);
	gtkLabelSetColor (CAN_LABEL_BOOL_TO_LABEL (page->inverterMaxTemp), &FONT_COLOR);
	gtk_widget_set_halign (CAN_WIDGET_TO_WIDGET (page->inverterMaxTemp), GTK_ALIGN_END);
	gtk_widget_set_margin_end (CAN_WIDGET_TO_WIDGET (page->inverterMaxTemp), 3);
	gtk_grid_attach (page->rightPanel, CAN_WIDGET_TO_WIDGET (page->inverterMaxTemp), 1, 3, 1, 1);

	label = gtk_label_new ("Mtr:");
	gtkLabelSetFont (GTK_LABEL (label), PANEL_STAT_FONT);
	gtkLabelSetColor (GTK_LABEL (label), &FONT_COLOR);
	gtk_widget_set_halign (label, GTK_ALIGN_START);
	gtk_widget_set_margin_start (label, 3);
	gtk_grid_attach (page->rightPanel, label, 0, 4, 1, 1);

	page->motorMaxTemp = canLabelFloatInit (database, &(canLabelFloatConfig_t)
	{
		.signalName		= "AMK_MOTOR_TEMPERATURE_MAX",
		.formatValue	= "%.1f %s",
		.formatInvalid	= "%s %s"
	});
	gtkLabelSetFont (CAN_LABEL_FLOAT_TO_LABEL (page->motorMaxTemp), PANEL_STAT_FONT);
	gtkLabelSetColor (CAN_LABEL_BOOL_TO_LABEL (page->motorMaxTemp), &FONT_COLOR);
	gtk_widget_set_halign (CAN_WIDGET_TO_WIDGET (page->motorMaxTemp), GTK_ALIGN_END);
	gtk_widget_set_margin_end (CAN_WIDGET_TO_WIDGET (page->motorMaxTemp), 3);
	gtk_grid_attach (page->rightPanel, CAN_WIDGET_TO_WIDGET (page->motorMaxTemp), 1, 4, 1, 1);

	page->buttonPanel = GTK_GRID (gtk_grid_new ());
	gtk_widget_set_margin_top (GTK_WIDGET (page->buttonPanel), 50);
	gtk_grid_attach (page->grid, GTK_WIDGET (page->buttonPanel), 2, 3, 3, 1);

	page->apps = canProgressBarInit (database, &(canProgressBarConfig_t)
	{
		.signalName = "APPS_1_PERCENT",
		.min		= 0,
		.max		= 100
	});
	gtk_orientable_set_orientation (CAN_PROGRESS_BAR_TO_ORIENTABLE (page->apps), GTK_ORIENTATION_VERTICAL);
	gtk_progress_bar_set_inverted (CAN_PROGRESS_BAR_TO_PROGRESS_BAR (page->apps), true);
	gtk_widget_set_vexpand (CAN_WIDGET_TO_WIDGET (page->apps), true);
	gtk_widget_set_size_request (CAN_WIDGET_TO_WIDGET (page->apps), 20, 0);
	gtk_grid_attach (page->grid, CAN_WIDGET_TO_WIDGET (page->apps), 5, 0, 1, 5);

	// Return the created page (cast to the base type).
	return (page_t*) page;
}

void pageAutoxAppendButton (void* page, const char* label, pageButtonCallback_t* callback, void* arg, bool currentPage)
{
	pageAutox_t* pageAutox = page;

	stylizedButton_t* button = stylizedButtonInit (callback, arg, &(stylizedButtonConfig_t)
	{
		.width				= 120,
		.height				= 80,
		.label				= label,
		.borderWidth		= 1,
		.backgroundColor	= gdkHexToColor ("#000000"),
		.borderColor		= gdkHexToColor ("#D3792C"),
		.selectedColor		= gdkHexToColor ("#F4931E"),
		.indicatorColor		= currentPage ? gdkHexToColor ("#FF0000") : gdkHexToColor ("#580000")
	});
	gtkLabelSetFont (STYLIZED_BUTTON_TO_LABEL (button), BUTTON_LABEL_FONT);
	gtk_widget_set_margin_top (STYLIZED_BUTTON_TO_WIDGET (button), 8);
	gtk_widget_set_margin_start (STYLIZED_BUTTON_TO_WIDGET (button), 4);
	gtk_widget_set_margin_end (STYLIZED_BUTTON_TO_WIDGET (button), 4);
	gtk_widget_set_hexpand (STYLIZED_BUTTON_TO_WIDGET (button), true);
	gtk_grid_attach (GTK_GRID (pageAutox->buttonPanel), STYLIZED_BUTTON_TO_WIDGET (button), pageAutox->buttonCount, 0, 1, 1);
	++pageAutox->buttonCount;
}

void pageAutoxUpdate (void* page)
{
	pageAutox_t* pageAutox = page;

	canWidgetUpdate (pageAutox->bse);
	canWidgetUpdate (pageAutox->apps);
	canWidgetUpdate (pageAutox->dataLoggerStatus);
	canWidgetUpdate (pageAutox->dataLoggerSession);
	canWidgetUpdate (pageAutox->glvVoltage);
	canWidgetUpdate (pageAutox->hvVoltage);
	canWidgetUpdate (pageAutox->inverterMaxTemp);
	canWidgetUpdate (pageAutox->motorMaxTemp);
	canWidgetUpdate (pageAutox->drivingTorque);
	canWidgetUpdate (pageAutox->regenTorque);
	canWidgetUpdate (pageAutox->torqueIndex);
	canWidgetUpdate (pageAutox->speed);
	canWidgetUpdate (pageAutox->drsStatus);
	canWidgetUpdate (pageAutox->vcuFault);
	canWidgetUpdate (pageAutox->bmsFault);
	canWidgetUpdate (pageAutox->amkFault);
	canWidgetUpdate (pageAutox->gpsFault);
}
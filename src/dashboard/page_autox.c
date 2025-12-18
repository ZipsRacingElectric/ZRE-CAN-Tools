// Header
#include "page_autox.h"

// Includes
#include "gtk_util.h"

#define CENTER_TITLE_FONT	"ITC Avant Garde Gothic CE Book 22px"
#define CENTER_VALUE_FONT	"Technology Bold 200px"
#define PANEL_TITLE_FONT	"Futura Std Bold Condensed 26px"
#define PANEL_STAT_FONT		"ITC Avant Garde Gothic CE Book 26px"

static void drsDraw (GtkDrawingArea* area, cairo_t* cairo, int width, int height, gpointer data)
{
	(void) area;
	canIndicator_t* icon = data;

	float value = (icon->active && icon->valid) * 0.75f + 0.25f;
	GdkRGBA color =
	{
		.red	= value,
		.green	= value,
		.blue	= value,
		.alpha	= 1
	};

	cairo_arc (cairo, width / 2.0, height / 2.0, MIN (width, height) / 2.0, 0, 2 * G_PI);
	gdk_cairo_set_source_rgba (cairo, &color);
	cairo_fill (cairo);
}

void pageAutoxInit (pageAutox_t* page, canDatabase_t* database)
{
	// Create the top-level grid
	page->widget = gtk_grid_new ();

	// BSE bar
	page->bse = (canProgressBar_t)
	{
		.signalName = "BSE_FRONT_PERCENT",
		.min		= 0,
		.max		= 100
	};
	canProgessBarInit (&page->bse, database);
	gtk_orientable_set_orientation (GTK_ORIENTABLE (CAN_PROGRESS_BAR_TO_WIDGET (&page->bse)), GTK_ORIENTATION_VERTICAL);
	gtk_progress_bar_set_inverted (CAN_PROGRESS_BAR_TO_PROGRESS_BAR (&page->bse), true);
	gtk_grid_attach (GTK_GRID (page->widget), CAN_PROGRESS_BAR_TO_WIDGET (&page->bse), 0, 0, 1, 2);

	GtkWidget* subGrid = gtk_grid_new ();
	gtk_grid_attach (GTK_GRID (page->widget), subGrid, 1, 0, 1, 1);

	GtkWidget* frame = gtk_frame_new ("");
	gtk_grid_attach (GTK_GRID (subGrid), frame, 0, 1, 1, 1);
	gtk_widget_set_vexpand (frame, true);
	gtk_widget_set_size_request (frame, 50, 0);

	frame = gtk_frame_new ("Data Logger");
	gtk_grid_attach (GTK_GRID (subGrid), frame, 0, 0, 2, 1);
	gtk_widget_set_size_request (frame, 200, 150);

	GtkWidget* leftPanel = gtk_grid_new ();
	gtk_widget_set_margin_start (leftPanel, 10);
	gtk_widget_set_margin_end (leftPanel, 10);
	gtk_grid_attach (GTK_GRID (subGrid), leftPanel, 1, 1, 1, 1);

	GtkWidget* label = gtk_label_new ("Torque Config");
	gtkLabelSetFont (GTK_LABEL (label), PANEL_TITLE_FONT);
	gtk_widget_set_size_request (label, 200, 0);
	gtk_label_set_xalign (GTK_LABEL (label), 0.5);
	gtk_grid_attach (GTK_GRID (leftPanel), label, 0, 0, 2, 1);

	label = gtk_label_new ("Driving:");
	gtkLabelSetFont (GTK_LABEL (label), PANEL_STAT_FONT);
	gtk_widget_set_halign (label, GTK_ALIGN_START);
	gtk_grid_attach (GTK_GRID (leftPanel), label, 0, 1, 1, 1);

	page->drivingTorque = (canLabelFloat_t)
	{
		.signalName		= "DRIVING_TORQUE_LIMIT",
		.formatValue	= "%.1f%s",
		.formatInvalid	= "%s%s"
	};
	canLabelFloatInit (&page->drivingTorque, database);
	gtkLabelSetFont (CAN_LABEL_TO_LABEL (&page->drivingTorque), PANEL_STAT_FONT);
	gtk_widget_set_halign (CAN_LABEL_TO_WIDGET (&page->drivingTorque), GTK_ALIGN_END);
	gtk_grid_attach (GTK_GRID (leftPanel), CAN_LABEL_TO_WIDGET (&page->drivingTorque), 1, 1, 1, 1);

	label = gtk_label_new ("Regen:");
	gtkLabelSetFont (GTK_LABEL (label), PANEL_STAT_FONT);
	gtk_widget_set_halign (label, GTK_ALIGN_START);
	gtk_grid_attach (GTK_GRID (leftPanel), label, 0, 2, 1, 1);

	page->regenTorque = (canLabelFloat_t)
	{
		.signalName		= "REGEN_TORQUE_LIMIT",
		.formatValue	= "%.1f%s",
		.formatInvalid	= "%s%s"
	};
	canLabelFloatInit (&page->regenTorque, database);
	gtkLabelSetFont (CAN_LABEL_TO_LABEL (&page->regenTorque), PANEL_STAT_FONT);
	gtk_widget_set_halign (CAN_LABEL_TO_WIDGET (&page->regenTorque), GTK_ALIGN_END);
	gtk_grid_attach (GTK_GRID (leftPanel), CAN_LABEL_TO_WIDGET (&page->regenTorque), 1, 2, 1, 1);

	label = gtk_label_new ("Index:");
	gtkLabelSetFont (GTK_LABEL (label), PANEL_STAT_FONT);
	gtk_widget_set_halign (label, GTK_ALIGN_START);
	gtk_grid_attach (GTK_GRID (leftPanel), label, 0, 3, 1, 1);

	page->torqueIndex = (canLabelFloat_t)
	{
		.signalName		= "TORQUE_ALGORITHM_INDEX",
		.formatValue	= "%.1f",
		.formatInvalid	= "%s"
	};
	canLabelFloatInit (&page->torqueIndex, database);
	gtkLabelSetFont (CAN_LABEL_TO_LABEL (&page->torqueIndex), PANEL_STAT_FONT);
	gtk_widget_set_halign (CAN_LABEL_TO_WIDGET (&page->torqueIndex), GTK_ALIGN_END);
	gtk_grid_attach (GTK_GRID (leftPanel), CAN_LABEL_TO_WIDGET (&page->torqueIndex), 1, 3, 1, 1);

	label = gtk_label_new ("DRS:");
	gtkLabelSetFont (GTK_LABEL (label), PANEL_STAT_FONT);
	gtk_widget_set_halign (label, GTK_ALIGN_START);
	gtk_grid_attach (GTK_GRID (leftPanel), label, 0, 4, 1, 1);

	page->drsStatus = (canIndicator_t)
	{
		.signalName = "APPS_1_PERCENT",
		.threshold	= 25,
		.inverted	= true
	};
	canIndicatorInit (&page->drsStatus, database, drsDraw, 26, 26);
	gtk_widget_set_halign (CAN_INDICATOR_TO_WIDGET (&page->drsStatus), GTK_ALIGN_END);
	gtk_grid_attach (GTK_GRID (leftPanel), CAN_INDICATOR_TO_WIDGET (&page->drsStatus), 1, 4, 1, 1);

	subGrid = gtk_grid_new ();
	gtk_grid_attach (GTK_GRID (page->widget), subGrid, 2, 0, 1, 1);

	frame = gtk_frame_new ("");
	gtk_grid_attach (GTK_GRID (subGrid), frame, 0, 0, 1, 1);
	gtk_widget_set_size_request (frame, 0, 100);

	GtkWidget* centerPanel = gtk_grid_new ();
	gtk_grid_attach (GTK_GRID (subGrid), centerPanel, 0, 1, 1, 1);

	label = gtk_label_new ("Vehicle Speed (Km/h):");
	gtkLabelSetFont (GTK_LABEL (label), CENTER_TITLE_FONT);
	gtk_label_set_xalign (GTK_LABEL (label), 0);
	gtk_grid_attach (GTK_GRID (centerPanel), label, 0, 0, 1, 1);

	page->speed = (canLabelFloat_t)
	{
		.signalName		= "SPEED",
		.formatValue	= "%.0f",
		.formatInvalid	= "%s"
	};
	canLabelFloatInit (&page->speed, database);
	gtkLabelSetFont (CAN_LABEL_TO_LABEL (&page->speed), CENTER_VALUE_FONT);
	gtk_widget_set_hexpand (CAN_LABEL_TO_WIDGET (&page->speed), true);
	gtk_widget_set_vexpand (CAN_LABEL_TO_WIDGET (&page->speed), true);
	gtk_grid_attach (GTK_GRID (centerPanel), CAN_LABEL_TO_WIDGET (&page->speed), 0, 1, 1, 1);

	GtkWidget* rightPanel = gtk_grid_new ();
	gtk_widget_set_margin_start (rightPanel, 10);
	gtk_widget_set_margin_end (rightPanel, 10);
	gtk_grid_attach (GTK_GRID (page->widget), rightPanel, 3, 0, 1, 1);
	// gtk_widget_set_size_request (rightPanel, 200, 0);

	frame = gtk_frame_new ("");
	gtk_widget_set_size_request (frame, 0, 150);
	gtk_grid_attach (GTK_GRID (rightPanel), frame, 0, 0, 2, 1);
	label = gtk_label_new ("Powertrain");
	gtkLabelSetFont (GTK_LABEL (label), PANEL_TITLE_FONT);
	// gtk_widget_set_halign (label, GTK_ALIGN_FILL);
	gtk_widget_set_size_request (label, 200, 0);
	gtk_label_set_xalign (GTK_LABEL (label), 0.5);
	gtk_grid_attach (GTK_GRID (rightPanel), label, 0, 1, 2, 1);
	// gtk_widget_set_hexpand (label, true);

	label = gtk_label_new ("GLV:");
	gtkLabelSetFont (GTK_LABEL (label), PANEL_STAT_FONT);
	gtk_widget_set_halign (label, GTK_ALIGN_START);
	gtk_grid_attach (GTK_GRID (rightPanel), label, 0, 2, 1, 1);

	page->glvVoltage = (canLabelFloat_t)
	{
		.signalName		= "GLV_BATTERY_VOLTAGE",
		.formatValue	= "%.1f %s",
		.formatInvalid	= "%s %s"
	};
	canLabelFloatInit (&page->glvVoltage, database);
	gtkLabelSetFont (CAN_LABEL_TO_LABEL (&page->glvVoltage), PANEL_STAT_FONT);
	gtk_widget_set_halign (CAN_LABEL_TO_WIDGET (&page->glvVoltage), GTK_ALIGN_END);
	gtk_grid_attach (GTK_GRID (rightPanel), CAN_LABEL_TO_WIDGET (&page->glvVoltage), 1, 2, 1, 1);

	label = gtk_label_new ("HV:");
	gtkLabelSetFont (GTK_LABEL (label), PANEL_STAT_FONT);
	gtk_widget_set_halign (label, GTK_ALIGN_START);
	gtk_grid_attach (GTK_GRID (rightPanel), label, 0, 3, 1, 1);

	page->hvVoltage = (canLabelFloat_t)
	{
		.signalName		= "PACK_VOLTAGE",
		.formatValue	= "%.1f %s",
		.formatInvalid	= "%s %s"
	};
	canLabelFloatInit (&page->hvVoltage, database);
	gtkLabelSetFont (CAN_LABEL_TO_LABEL (&page->hvVoltage), PANEL_STAT_FONT);
	gtk_widget_set_halign (CAN_LABEL_TO_WIDGET (&page->hvVoltage), GTK_ALIGN_END);
	gtk_grid_attach (GTK_GRID (rightPanel), CAN_LABEL_TO_WIDGET (&page->hvVoltage), 1, 3, 1, 1);

	label = gtk_label_new ("Inv:");
	gtkLabelSetFont (GTK_LABEL (label), PANEL_STAT_FONT);
	gtk_widget_set_halign (label, GTK_ALIGN_START);
	gtk_grid_attach (GTK_GRID (rightPanel), label, 0, 4, 1, 1);

	page->inverterMaxTemp = (canLabelFloat_t)
	{
		.signalName		= "TODO",
		.formatValue	= "%.1f %s",
		.formatInvalid	= "%s %s"
	};
	canLabelFloatInit (&page->inverterMaxTemp, database);
	gtkLabelSetFont (CAN_LABEL_TO_LABEL (&page->inverterMaxTemp), PANEL_STAT_FONT);
	gtk_widget_set_halign (CAN_LABEL_TO_WIDGET (&page->inverterMaxTemp), GTK_ALIGN_END);
	gtk_grid_attach (GTK_GRID (rightPanel), CAN_LABEL_TO_WIDGET (&page->inverterMaxTemp), 1, 4, 1, 1);

	label = gtk_label_new ("Mtr:");
	gtkLabelSetFont (GTK_LABEL (label), PANEL_STAT_FONT);
	gtk_widget_set_halign (label, GTK_ALIGN_START);
	gtk_grid_attach (GTK_GRID (rightPanel), label, 0, 5, 1, 1);

	page->motorMaxTemp = (canLabelFloat_t)
	{
		.signalName		= "TODO",
		.formatValue	= "%.1f %s",
		.formatInvalid	= "%s %s"
	};
	canLabelFloatInit (&page->motorMaxTemp, database);
	gtkLabelSetFont (CAN_LABEL_TO_LABEL (&page->motorMaxTemp), PANEL_STAT_FONT);
	gtk_widget_set_halign (CAN_LABEL_TO_WIDGET (&page->motorMaxTemp), GTK_ALIGN_END);
	gtk_grid_attach (GTK_GRID (rightPanel), CAN_LABEL_TO_WIDGET (&page->motorMaxTemp), 1, 5, 1, 1);

	subGrid = gtk_grid_new ();
	gtk_grid_attach (GTK_GRID (page->widget), subGrid, 1, 1, 3, 1);

	frame = gtk_frame_new ("");
	gtk_grid_attach (GTK_GRID (subGrid), frame, 0, 0, 1, 1);
	gtk_widget_set_size_request (frame, 50, 0);

	GtkWidget* button = gtk_button_new ();
	gtk_grid_attach (GTK_GRID (subGrid), button, 1, 0, 1, 1);
	gtk_widget_set_hexpand (button, true);

	button = gtk_button_new ();
	gtk_grid_attach (GTK_GRID (subGrid), button, 2, 0, 1, 1);
	gtk_widget_set_hexpand (button, true);

	button = gtk_button_new ();
	gtk_grid_attach (GTK_GRID (subGrid), button, 3, 0, 1, 1);
	gtk_widget_set_hexpand (button, true);

	button = gtk_button_new ();
	gtk_grid_attach (GTK_GRID (subGrid), button, 4, 0, 1, 1);
	gtk_widget_set_hexpand (button, true);

	button = gtk_button_new ();
	gtk_grid_attach (GTK_GRID (subGrid), button, 5, 0, 1, 1);
	gtk_widget_set_hexpand (button, true);
	gtk_widget_set_size_request (button, 0, 90);

	page->apps = (canProgressBar_t)
	{
		.signalName = "APPS_1_PERCENT",
		.min		= 0,
		.max		= 100
	};
	canProgessBarInit (&page->apps, database);
	gtk_orientable_set_orientation (GTK_ORIENTABLE (CAN_PROGRESS_BAR_TO_WIDGET (&page->apps)), GTK_ORIENTATION_VERTICAL);
	gtk_progress_bar_set_inverted (CAN_PROGRESS_BAR_TO_PROGRESS_BAR (&page->apps), true);
	gtk_grid_attach (GTK_GRID (page->widget), CAN_PROGRESS_BAR_TO_WIDGET (&page->apps), 4, 0, 1, 2);
}

void pageAutoxUpdate (pageAutox_t* page)
{
	canProgressBarUpdate (&page->bse);
	canProgressBarUpdate (&page->apps);
	canLabelFloatUpdate (&page->glvVoltage);
	canLabelFloatUpdate (&page->hvVoltage);
	canLabelFloatUpdate (&page->inverterMaxTemp);
	canLabelFloatUpdate (&page->motorMaxTemp);
	canLabelFloatUpdate (&page->drivingTorque);
	canLabelFloatUpdate (&page->regenTorque);
	canLabelFloatUpdate (&page->torqueIndex);
	canLabelFloatUpdate (&page->speed);
	canIndicatorUpdate (&page->drsStatus);
}
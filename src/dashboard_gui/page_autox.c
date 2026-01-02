// Header
#include "page_autox.h"

// Includes
#include "gtk_util.h"

#define BUTTON_LABEL_FONT		"Futura Std Bold Condensed 26px"
#define DATA_LOGGER_TITLE_FONT	"Futura Std Bold Condensed 36px"
#define DATA_LOGGER_STAT_FONT	"ITC Avant Garde Gothic CE Book 26px"
#define CENTER_TITLE_FONT		"ITC Avant Garde Gothic CE Book 22px"
#define CENTER_VALUE_FONT		"Technology Bold 200px"
#define PANEL_TITLE_FONT		"Futura Std Bold Condensed 26px"
#define PANEL_STAT_FONT			"ITC Avant Garde Gothic CE Book 26px"
#define FAULT_NAME_FONT			"Futura Std Bold Condensed 32px @color=#000000"

page_t* pageAutoxInit (canDatabase_t* database)
{
	// Allocate the page
	pageAutox_t* page = malloc (sizeof (pageAutox_t));
	if (page == NULL)
		return NULL;

	// Setup the VMT
	page->vmt.update = pageAutoxUpdate;
	page->vmt.appendButton = pageAutoxAppendButton;
	page->vmt.widget = gtk_grid_new ();
	page->vmt.buttonCount = 0;

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
	gtk_widget_set_vexpand (CAN_PROGRESS_BAR_TO_WIDGET (&page->bse), true);
	gtk_widget_set_size_request (CAN_PROGRESS_BAR_TO_WIDGET (&page->bse), 20, 0);
	gtk_grid_attach (GTK_GRID (page->vmt.widget), CAN_PROGRESS_BAR_TO_WIDGET (&page->bse), 0, 0, 1, 6);

	GtkWidget* dataLoggerPanel = gtk_grid_new ();
	gtk_grid_attach (GTK_GRID (page->vmt.widget), dataLoggerPanel, 1, 1, 2, 2);

	page->dataLoggerStatus = (canLabelBool_t)
	{
		.signalName		= "MIN_BUS_LOAD",
		.activeValue	= "Logging: On",
		.inactiveValue	= "Logging: On",
		.invalidValue	= "Logging: Off",
		.threshold		= 0,
		.inverted		= true
	};
	canLabelBoolInit (&page->dataLoggerStatus, database);
	gtkLabelSetFont (CAN_LABEL_BOOL_TO_LABEL (&page->dataLoggerStatus), DATA_LOGGER_TITLE_FONT);
	gtk_label_set_xalign (CAN_LABEL_BOOL_TO_LABEL (&page->dataLoggerStatus), 0);
	gtk_grid_attach (GTK_GRID (dataLoggerPanel), CAN_LABEL_BOOL_TO_WIDGET (&page->dataLoggerStatus), 0, 0, 1, 1);

	page->dataLoggerSession = (canLabelFloat_t)
	{
		.signalName		= "SESSION_NUMBER",
		.formatValue	= "Session\nNo. %.0f",
		.formatInvalid	= "Session\nNo. %s",
	};
	canLabelFloatInit (&page->dataLoggerSession, database);
	gtkLabelSetFont (CAN_LABEL_FLOAT_TO_LABEL (&page->dataLoggerSession), DATA_LOGGER_STAT_FONT);
	gtk_label_set_xalign (CAN_LABEL_BOOL_TO_LABEL (&page->dataLoggerSession), 0);
	gtk_grid_attach (GTK_GRID (dataLoggerPanel), CAN_LABEL_FLOAT_TO_WIDGET (&page->dataLoggerSession), 0, 1, 1, 1);

	GtkWidget* padding = gtk_grid_new ();
	gtk_widget_set_size_request (padding, 80, 0);
	gtk_grid_attach (GTK_GRID (page->vmt.widget), padding, 1, 3, 1, 1);

	GtkWidget* leftPanel = gtk_grid_new ();
	gtk_widget_set_margin_start (leftPanel, 10);
	gtk_widget_set_margin_end (leftPanel, 10);
	gtk_widget_set_valign (leftPanel, GTK_ALIGN_CENTER);
	gtk_grid_attach (GTK_GRID (page->vmt.widget), leftPanel, 2, 3, 1, 1);

	GtkWidget* label = gtk_label_new ("Torque Config");
	gtkLabelSetFont (GTK_LABEL (label), PANEL_TITLE_FONT);
	gtk_widget_set_size_request (label, 180, 0);
	gtk_label_set_xalign (GTK_LABEL (label), 0.5);
	gtk_grid_attach (GTK_GRID (leftPanel), label, 0, 0, 2, 1);

	label = gtk_label_new ("Driving:");
	gtkLabelSetFont (GTK_LABEL (label), PANEL_STAT_FONT);
	gtk_widget_set_halign (label, GTK_ALIGN_START);
	gtk_grid_attach (GTK_GRID (leftPanel), label, 0, 1, 1, 1);

	page->drivingTorque = (canLabelFloat_t)
	{
		.signalName		= "DRIVING_TORQUE_LIMIT",
		.formatValue	= "%.0f%s",
		.formatInvalid	= "%s%s"
	};
	canLabelFloatInit (&page->drivingTorque, database);
	gtkLabelSetFont (CAN_LABEL_FLOAT_TO_LABEL (&page->drivingTorque), PANEL_STAT_FONT);
	gtk_widget_set_halign (CAN_LABEL_FLOAT_TO_WIDGET (&page->drivingTorque), GTK_ALIGN_END);
	gtk_grid_attach (GTK_GRID (leftPanel), CAN_LABEL_FLOAT_TO_WIDGET (&page->drivingTorque), 1, 1, 1, 1);

	label = gtk_label_new ("Regen:");
	gtkLabelSetFont (GTK_LABEL (label), PANEL_STAT_FONT);
	gtk_widget_set_halign (label, GTK_ALIGN_START);
	gtk_grid_attach (GTK_GRID (leftPanel), label, 0, 2, 1, 1);

	page->regenTorque = (canLabelFloat_t)
	{
		.signalName		= "REGEN_TORQUE_LIMIT",
		.formatValue	= "%.0f%s",
		.formatInvalid	= "%s%s"
	};
	canLabelFloatInit (&page->regenTorque, database);
	gtkLabelSetFont (CAN_LABEL_FLOAT_TO_LABEL (&page->regenTorque), PANEL_STAT_FONT);
	gtk_widget_set_halign (CAN_LABEL_FLOAT_TO_WIDGET (&page->regenTorque), GTK_ALIGN_END);
	gtk_grid_attach (GTK_GRID (leftPanel), CAN_LABEL_FLOAT_TO_WIDGET (&page->regenTorque), 1, 2, 1, 1);

	label = gtk_label_new ("Index:");
	gtkLabelSetFont (GTK_LABEL (label), PANEL_STAT_FONT);
	gtk_widget_set_halign (label, GTK_ALIGN_START);
	gtk_grid_attach (GTK_GRID (leftPanel), label, 0, 3, 1, 1);

	page->torqueIndex = (canLabelFloat_t)
	{
		.signalName		= "TORQUE_ALGORITHM_INDEX",
		.formatValue	= "%.0f",
		.formatInvalid	= "%s"
	};
	canLabelFloatInit (&page->torqueIndex, database);
	gtkLabelSetFont (CAN_LABEL_FLOAT_TO_LABEL (&page->torqueIndex), PANEL_STAT_FONT);
	gtk_widget_set_halign (CAN_LABEL_FLOAT_TO_WIDGET (&page->torqueIndex), GTK_ALIGN_END);
	gtk_grid_attach (GTK_GRID (leftPanel), CAN_LABEL_FLOAT_TO_WIDGET (&page->torqueIndex), 1, 3, 1, 1);

	label = gtk_label_new ("DRS:");
	gtkLabelSetFont (GTK_LABEL (label), PANEL_STAT_FONT);
	gtk_widget_set_halign (label, GTK_ALIGN_START);
	gtk_grid_attach (GTK_GRID (leftPanel), label, 0, 4, 1, 1);

	page->drsStatus = (canIndicator_t)
	{
		.signalName 	= "TODO",
		.threshold		= 25,
		.inverted		= true,
		.points			= NULL,
		.pointCount		= 0,
		.activeColor	= {1, 1, 1, 1},
		.inactiveColor	= {0.25f, 0.25f, 0.25f, 1},
		.invalidColor	= {1, 0, 0, 1}
	};

	canIndicatorInit (&page->drsStatus, database, 26, 26);
	gtk_widget_set_halign (CAN_INDICATOR_TO_WIDGET (&page->drsStatus), GTK_ALIGN_END);
	gtk_grid_attach (GTK_GRID (leftPanel), CAN_INDICATOR_TO_WIDGET (&page->drsStatus), 1, 4, 1, 1);

	GtkWidget* faultPanel = gtk_grid_new ();
	gtk_widget_set_margin_top (faultPanel, 10);
	gtk_widget_set_margin_bottom (faultPanel, 10);
	gtk_grid_attach (GTK_GRID (page->vmt.widget), faultPanel, 3, 0, 2, 2);

	static float points [][2] =
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

	page->vcuFault = (canIndicator_t)
	{
		.signalName	= "VCU_FAULT",
		.threshold	= 0.5f,
		.inverted	= false,
		.points		= points,
		.pointCount	= sizeof (points) / sizeof (points [0]),
		.activeColor	= {1, 0, 0, 1},
		.inactiveColor	= {0.25f, 0.25f, 0.25f, 1},
		.invalidColor	= {1, 0, 0, 1}
	};
	canIndicatorInit (&page->vcuFault, database, 100, 42);
	gtk_widget_set_hexpand (CAN_INDICATOR_TO_WIDGET (&page->vcuFault), true);
	gtk_grid_attach (GTK_GRID (faultPanel), CAN_INDICATOR_TO_WIDGET (&page->vcuFault), 0, 0, 1, 1);

	label = gtk_label_new ("VCU");
	gtkLabelSetFont (GTK_LABEL (label), FAULT_NAME_FONT);
	gtkLabelSetColor (GTK_LABEL (label), "#000000");
	gtk_grid_attach (GTK_GRID (faultPanel), label, 0, 0, 1, 1);

	page->bmsFault = (canIndicator_t)
	{
		.signalName	= "BMS_FAULT",
		.threshold	= 0.5f,
		.inverted	= false,
		.points		= points,
		.pointCount	= sizeof (points) / sizeof (points [0]),
		.activeColor	= {1, 0, 0, 1},
		.inactiveColor	= {0.25f, 0.25f, 0.25f, 1},
		.invalidColor	= {1, 0, 0, 1}
	};
	canIndicatorInit (&page->bmsFault, database, 100, 42);
	gtk_widget_set_hexpand (CAN_INDICATOR_TO_WIDGET (&page->bmsFault), true);
	gtk_grid_attach (GTK_GRID (faultPanel), CAN_INDICATOR_TO_WIDGET (&page->bmsFault), 1, 0, 1, 1);

	label = gtk_label_new ("BMS");
	gtkLabelSetFont (GTK_LABEL (label), FAULT_NAME_FONT);
	gtkLabelSetColor (GTK_LABEL (label), "#000000");
	gtk_grid_attach (GTK_GRID (faultPanel), label, 1, 0, 1, 1);

	page->amkFault = (canIndicator_t)
	{
		.signalName	= "AMK_FAULT",
		.threshold	= 0.5f,
		.inverted	= false,
		.points		= points,
		.pointCount	= sizeof (points) / sizeof (points [0]),
		.activeColor	= {1, 0, 0, 1},
		.inactiveColor	= {0.25f, 0.25f, 0.25f, 1},
		.invalidColor	= {1, 0, 0, 1}
	};
	canIndicatorInit (&page->amkFault, database, 100, 42);
	gtk_widget_set_hexpand (CAN_INDICATOR_TO_WIDGET (&page->amkFault), true);
	gtk_grid_attach (GTK_GRID (faultPanel), CAN_INDICATOR_TO_WIDGET (&page->amkFault), 2, 0, 1, 1);

	label = gtk_label_new ("AMK");
	gtkLabelSetFont (GTK_LABEL (label), FAULT_NAME_FONT);
	gtkLabelSetColor (GTK_LABEL (label), "#000000");
	gtk_grid_attach (GTK_GRID (faultPanel), label, 2, 0, 1, 1);

	page->gpsFault = (canIndicator_t)
	{
		.signalName	= "GPS_STATUS",
		.threshold	= 2.5f,
		.inverted	= true,
		.points		= points,
		.pointCount	= sizeof (points) / sizeof (points [0]),
		.activeColor	= {1, 0, 0, 1},
		.inactiveColor	= {0.25f, 0.25f, 0.25f, 1},
		.invalidColor	= {1, 0, 0, 1}
	};
	canIndicatorInit (&page->gpsFault, database, 100, 42);
	gtk_widget_set_hexpand (CAN_INDICATOR_TO_WIDGET (&page->gpsFault), true);
	gtk_grid_attach (GTK_GRID (faultPanel), CAN_INDICATOR_TO_WIDGET (&page->gpsFault), 3, 0, 1, 1);

	label = gtk_label_new ("GPS");
	gtkLabelSetFont (GTK_LABEL (label), FAULT_NAME_FONT);
	gtkLabelSetColor (GTK_LABEL (label), "#000000");
	gtk_grid_attach (GTK_GRID (faultPanel), label, 3, 0, 1, 1);

	label = gtk_label_new ("Vehicle Speed (Km/h):");
	gtkLabelSetFont (GTK_LABEL (label), CENTER_TITLE_FONT);
	gtk_label_set_xalign (GTK_LABEL (label), 0);
	gtk_grid_attach (GTK_GRID (page->vmt.widget), label, 3, 2, 1, 1);

	page->speed = (canLabelFloat_t)
	{
		.signalName		= "SPEED",
		.formatValue	= "%02.0f",
		.formatInvalid	= "XX"
	};
	canLabelFloatInit (&page->speed, database);
	gtkLabelSetFont (CAN_LABEL_FLOAT_TO_LABEL (&page->speed), CENTER_VALUE_FONT);
	gtk_widget_set_hexpand (CAN_LABEL_FLOAT_TO_WIDGET (&page->speed), true);
	gtk_widget_set_vexpand (CAN_LABEL_FLOAT_TO_WIDGET (&page->speed), true);
	gtk_grid_attach (GTK_GRID (page->vmt.widget), CAN_LABEL_FLOAT_TO_WIDGET (&page->speed), 3, 3, 1, 1);

	GtkWidget* rightPanel = gtk_grid_new ();
	gtk_widget_set_margin_start (rightPanel, 10);
	gtk_widget_set_margin_end (rightPanel, 10);
	gtk_widget_set_valign (rightPanel, GTK_ALIGN_CENTER);
	gtk_grid_attach (GTK_GRID (page->vmt.widget), rightPanel, 4, 3, 1, 1);

	label = gtk_label_new ("Powertrain");
	gtkLabelSetFont (GTK_LABEL (label), PANEL_TITLE_FONT);
	gtk_widget_set_size_request (label, 180, 0);
	gtk_label_set_xalign (GTK_LABEL (label), 0.5);
	gtk_grid_attach (GTK_GRID (rightPanel), label, 0, 0, 2, 1);

	label = gtk_label_new ("GLV:");
	gtkLabelSetFont (GTK_LABEL (label), PANEL_STAT_FONT);
	gtk_widget_set_halign (label, GTK_ALIGN_START);
	gtk_grid_attach (GTK_GRID (rightPanel), label, 0, 1, 1, 1);

	page->glvVoltage = (canLabelFloat_t)
	{
		.signalName		= "GLV_BATTERY_VOLTAGE",
		.formatValue	= "%.1f %s",
		.formatInvalid	= "%s %s"
	};
	canLabelFloatInit (&page->glvVoltage, database);
	gtkLabelSetFont (CAN_LABEL_FLOAT_TO_LABEL (&page->glvVoltage), PANEL_STAT_FONT);
	gtk_widget_set_halign (CAN_LABEL_FLOAT_TO_WIDGET (&page->glvVoltage), GTK_ALIGN_END);
	gtk_grid_attach (GTK_GRID (rightPanel), CAN_LABEL_FLOAT_TO_WIDGET (&page->glvVoltage), 1, 1, 1, 1);

	label = gtk_label_new ("HV:");
	gtkLabelSetFont (GTK_LABEL (label), PANEL_STAT_FONT);
	gtk_widget_set_halign (label, GTK_ALIGN_START);
	gtk_grid_attach (GTK_GRID (rightPanel), label, 0, 2, 1, 1);

	page->hvVoltage = (canLabelFloat_t)
	{
		.signalName		= "PACK_VOLTAGE",
		.formatValue	= "%.1f %s",
		.formatInvalid	= "%s %s"
	};
	canLabelFloatInit (&page->hvVoltage, database);
	gtkLabelSetFont (CAN_LABEL_FLOAT_TO_LABEL (&page->hvVoltage), PANEL_STAT_FONT);
	gtk_widget_set_halign (CAN_LABEL_FLOAT_TO_WIDGET (&page->hvVoltage), GTK_ALIGN_END);
	gtk_grid_attach (GTK_GRID (rightPanel), CAN_LABEL_FLOAT_TO_WIDGET (&page->hvVoltage), 1, 2, 1, 1);

	label = gtk_label_new ("Inv:");
	gtkLabelSetFont (GTK_LABEL (label), PANEL_STAT_FONT);
	gtk_widget_set_halign (label, GTK_ALIGN_START);
	gtk_grid_attach (GTK_GRID (rightPanel), label, 0, 3, 1, 1);

	page->inverterMaxTemp = (canLabelFloat_t)
	{
		.signalName		= "AMK_INVERTER_TEMPERATURE_MAX",
		.formatValue	= "%.1f %s",
		.formatInvalid	= "%s %s"
	};
	canLabelFloatInit (&page->inverterMaxTemp, database);
	gtkLabelSetFont (CAN_LABEL_FLOAT_TO_LABEL (&page->inverterMaxTemp), PANEL_STAT_FONT);
	gtk_widget_set_halign (CAN_LABEL_FLOAT_TO_WIDGET (&page->inverterMaxTemp), GTK_ALIGN_END);
	gtk_grid_attach (GTK_GRID (rightPanel), CAN_LABEL_FLOAT_TO_WIDGET (&page->inverterMaxTemp), 1, 3, 1, 1);

	label = gtk_label_new ("Mtr:");
	gtkLabelSetFont (GTK_LABEL (label), PANEL_STAT_FONT);
	gtk_widget_set_halign (label, GTK_ALIGN_START);
	gtk_grid_attach (GTK_GRID (rightPanel), label, 0, 4, 1, 1);

	page->motorMaxTemp = (canLabelFloat_t)
	{
		.signalName		= "AMK_MOTOR_TEMPERATURE_MAX",
		.formatValue	= "%.1f %s",
		.formatInvalid	= "%s %s"
	};
	canLabelFloatInit (&page->motorMaxTemp, database);
	gtkLabelSetFont (CAN_LABEL_FLOAT_TO_LABEL (&page->motorMaxTemp), PANEL_STAT_FONT);
	gtk_widget_set_halign (CAN_LABEL_FLOAT_TO_WIDGET (&page->motorMaxTemp), GTK_ALIGN_END);
	gtk_grid_attach (GTK_GRID (rightPanel), CAN_LABEL_FLOAT_TO_WIDGET (&page->motorMaxTemp), 1, 4, 1, 1);

	page->vmt.buttonPanel = gtk_grid_new ();
	gtk_grid_attach (GTK_GRID (page->vmt.widget), page->vmt.buttonPanel, 2, 5, 3, 1);

	page->apps = (canProgressBar_t)
	{
		.signalName = "APPS_1_PERCENT",
		.min		= 0,
		.max		= 100
	};
	canProgessBarInit (&page->apps, database);
	gtk_orientable_set_orientation (GTK_ORIENTABLE (CAN_PROGRESS_BAR_TO_WIDGET (&page->apps)), GTK_ORIENTATION_VERTICAL);
	gtk_progress_bar_set_inverted (CAN_PROGRESS_BAR_TO_PROGRESS_BAR (&page->apps), true);
	gtk_widget_set_vexpand (CAN_PROGRESS_BAR_TO_WIDGET (&page->apps), true);
	gtk_widget_set_size_request (CAN_PROGRESS_BAR_TO_WIDGET (&page->apps), 20, 0);
	gtk_grid_attach (GTK_GRID (page->vmt.widget), CAN_PROGRESS_BAR_TO_WIDGET (&page->apps), 5, 0, 1, 6);

	// Return the created page (cast to the base type).
	return (page_t*) page;
}

void pageAutoxAppendButton (void* page, const char* label, pageButtonCallback_t* callback, void* arg)
{
	pageAutox_t* pageAutox = page;

	GtkWidget* button = gtk_button_new ();

	if (callback != NULL)
		g_signal_connect (button, "clicked", G_CALLBACK (callback), arg);

	GtkWidget* labelWidget = gtk_label_new (label);
	gtkLabelSetFont (GTK_LABEL (labelWidget), BUTTON_LABEL_FONT);
	gtk_button_set_child (GTK_BUTTON (button), labelWidget);

	gtk_widget_set_hexpand (button, true);
	gtk_widget_set_size_request (button, 120, 90);
	gtk_grid_attach (GTK_GRID (pageAutox->vmt.buttonPanel), button, pageAutox->vmt.buttonCount, 0, 1, 1);
	++pageAutox->vmt.buttonCount;
}

void pageAutoxUpdate (void* page)
{
	pageAutox_t* pageAutox = page;

	canProgressBarUpdate (&pageAutox->bse);
	canProgressBarUpdate (&pageAutox->apps);
	canLabelBoolUpdate (&pageAutox->dataLoggerStatus);
	canLabelFloatUpdate (&pageAutox->dataLoggerSession);
	canLabelFloatUpdate (&pageAutox->glvVoltage);
	canLabelFloatUpdate (&pageAutox->hvVoltage);
	canLabelFloatUpdate (&pageAutox->inverterMaxTemp);
	canLabelFloatUpdate (&pageAutox->motorMaxTemp);
	canLabelFloatUpdate (&pageAutox->drivingTorque);
	canLabelFloatUpdate (&pageAutox->regenTorque);
	canLabelFloatUpdate (&pageAutox->torqueIndex);
	canLabelFloatUpdate (&pageAutox->speed);
	canIndicatorUpdate (&pageAutox->drsStatus);
	canIndicatorUpdate (&pageAutox->vcuFault);
	canIndicatorUpdate (&pageAutox->bmsFault);
	canIndicatorUpdate (&pageAutox->amkFault);
	canIndicatorUpdate (&pageAutox->gpsFault);
}
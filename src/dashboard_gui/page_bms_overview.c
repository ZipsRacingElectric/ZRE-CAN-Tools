// Header
#include "page_bms_overview.h"

// Includes
#include "gtk_util.h"
#include "stylized_widgets/stylized_button.h"

#define STATUS_FONT				"Monospace 12px"
#define BUTTON_LABEL_FONT		"Futura Std Bold Condensed 34px"
#define PANEL_TITLE_FONT		"ITC Avant Garde Gothic CE Book 18px"

static void drawBg (GtkDrawingArea* area, cairo_t* cr, int width, int height, gpointer arg)
{
	pageBmsOverview_t* page = arg;
	(void) area;
	(void) arg;

	(void) page;

	cairo_rectangle (cr, 0, 0, width, height);
	cairo_fill (cr);
}

page_t* pageBmsOverviewInit (bms_t* bms)
{
	pageBmsOverview_t* page = malloc (sizeof (pageBmsOverview_t));
	if (page == NULL)
		return NULL;

	// Setup the VMT
	page->vmt.update = pageBmsOverviewUpdate;
	page->vmt.appendButton = pageBmsAppendButton;
	page->vmt.widget = gtk_overlay_new ();

	page->buttonCount = 0;

	page->bms = bms;

	GtkWidget* bg = gtk_drawing_area_new ();
	gtk_drawing_area_set_draw_func (GTK_DRAWING_AREA (bg), drawBg, page, NULL);
	gtk_overlay_set_child (GTK_OVERLAY (page->vmt.widget), bg);

	GtkWidget* grid = gtk_grid_new ();
	gtk_overlay_add_overlay (GTK_OVERLAY (page->vmt.widget), grid);

	GtkWidget* statusTitle = gtk_grid_new ();
	gtk_grid_attach (GTK_GRID (grid), statusTitle, 0, 0, 2, 1);

	GtkWidget* label = gtk_label_new ("BMS Status: ");
	gtk_grid_attach (GTK_GRID (statusTitle), label, 0, 0, 1, 1);
	gtkLabelSetFont (GTK_LABEL (label), STATUS_FONT);

	bmsFaultLabelInit (&page->faultLabel, bms);
	gtk_grid_attach (GTK_GRID (statusTitle), BMS_FAULT_LABEL_TO_WIDGET (&page->faultLabel), 1, 0, 2, 1);
	gtkLabelSetFont (BMS_FAULT_LABEL_TO_LABEL (&page->faultLabel), STATUS_FONT);

	GtkWidget* statusPanel = gtk_grid_new ();
	gtk_grid_attach (GTK_GRID (grid), statusPanel, 0, 1, 2, 1);

	label = gtk_label_new ("Voltage: ");
	page->voltageLabel = (canLabelFloatStatic_t)
	{
		.formatValue	= "%.1f%s",
		.formatInvalid	= "%s%s"
	};
	canLabelFloatStaticInit (&page->voltageLabel);
	gtkLabelSetFont (GTK_LABEL (label), STATUS_FONT);
	gtkLabelSetFont (CAN_LABEL_FLOAT_STATIC_TO_LABEL (&page->voltageLabel), STATUS_FONT);
	gtk_grid_attach (GTK_GRID (statusPanel), label, 0, 0, 1, 1);
	gtk_grid_attach (GTK_GRID (statusPanel), CAN_LABEL_FLOAT_STATIC_TO_WIDGET (&page->voltageLabel), 1, 0, 1, 1);
	gtk_widget_set_hexpand (label, true);
	gtk_widget_set_halign (label, GTK_ALIGN_END);
	gtk_widget_set_hexpand (CAN_LABEL_FLOAT_STATIC_TO_WIDGET (&page->voltageLabel), true);
	gtk_widget_set_halign (CAN_LABEL_FLOAT_STATIC_TO_WIDGET (&page->voltageLabel), GTK_ALIGN_START);

	label = gtk_label_new ("Current: ");
	page->currentLabel = (canLabelFloatStatic_t)
	{
		.formatValue	= "%.1f%s",
		.formatInvalid	= "%s%s"
	};
	canLabelFloatStaticInit (&page->currentLabel);
	gtkLabelSetFont (GTK_LABEL (label), STATUS_FONT);
	gtkLabelSetFont (CAN_LABEL_FLOAT_STATIC_TO_LABEL (&page->currentLabel), STATUS_FONT);
	gtk_grid_attach (GTK_GRID (statusPanel), label, 2, 0, 1, 1);
	gtk_grid_attach (GTK_GRID (statusPanel), CAN_LABEL_FLOAT_STATIC_TO_WIDGET (&page->currentLabel), 3, 0, 1, 1);
	gtk_widget_set_hexpand (label, true);
	gtk_widget_set_halign (label, GTK_ALIGN_END);
	gtk_widget_set_hexpand (CAN_LABEL_FLOAT_STATIC_TO_WIDGET (&page->currentLabel), true);
	gtk_widget_set_halign (CAN_LABEL_FLOAT_STATIC_TO_WIDGET (&page->currentLabel), GTK_ALIGN_START);

	label = gtk_label_new ("Power: ");
	page->powerLabel = (canLabelFloatStatic_t)
	{
		.formatValue	= "%.1f%s",
		.formatInvalid	= "%s%s"
	};
	canLabelFloatStaticInit (&page->powerLabel);
	gtkLabelSetFont (GTK_LABEL (label), STATUS_FONT);
	gtkLabelSetFont (CAN_LABEL_FLOAT_STATIC_TO_LABEL (&page->powerLabel), STATUS_FONT);
	gtk_grid_attach (GTK_GRID (statusPanel), label, 4, 0, 1, 1);
	gtk_grid_attach (GTK_GRID (statusPanel), CAN_LABEL_FLOAT_STATIC_TO_WIDGET (&page->powerLabel), 5, 0, 1, 1);
	gtk_widget_set_hexpand (label, true);
	gtk_widget_set_halign (label, GTK_ALIGN_END);
	gtk_widget_set_hexpand (CAN_LABEL_FLOAT_STATIC_TO_WIDGET (&page->powerLabel), true);
	gtk_widget_set_halign (CAN_LABEL_FLOAT_STATIC_TO_WIDGET (&page->powerLabel), GTK_ALIGN_START);

	label = gtk_label_new ("Min Cell: ");
	page->minCellLabel = (canLabelFloatStatic_t)
	{
		.formatValue	= "%.3f%s",
		.formatInvalid	= "%s%s"
	};
	canLabelFloatStaticInit (&page->minCellLabel);
	gtkLabelSetFont (GTK_LABEL (label), STATUS_FONT);
	gtkLabelSetFont (CAN_LABEL_FLOAT_STATIC_TO_LABEL (&page->minCellLabel), STATUS_FONT);
	gtk_grid_attach (GTK_GRID (statusPanel), label, 6, 0, 1, 1);
	gtk_grid_attach (GTK_GRID (statusPanel), CAN_LABEL_FLOAT_STATIC_TO_WIDGET (&page->minCellLabel), 7, 0, 1, 1);
	gtk_widget_set_hexpand (label, true);
	gtk_widget_set_halign (label, GTK_ALIGN_END);
	gtk_widget_set_hexpand (CAN_LABEL_FLOAT_STATIC_TO_WIDGET (&page->minCellLabel), true);
	gtk_widget_set_halign (CAN_LABEL_FLOAT_STATIC_TO_WIDGET (&page->minCellLabel), GTK_ALIGN_START);

	label = gtk_label_new ("Max Cell: ");
	page->maxCellLabel = (canLabelFloatStatic_t)
	{
		.formatValue	= "%.3f%s",
		.formatInvalid	= "%s%s"
	};
	canLabelFloatStaticInit (&page->maxCellLabel);
	gtkLabelSetFont (GTK_LABEL (label), STATUS_FONT);
	gtkLabelSetFont (CAN_LABEL_FLOAT_STATIC_TO_LABEL (&page->maxCellLabel), STATUS_FONT);
	gtk_grid_attach (GTK_GRID (statusPanel), label, 8, 0, 1, 1);
	gtk_grid_attach (GTK_GRID (statusPanel), CAN_LABEL_FLOAT_STATIC_TO_WIDGET (&page->maxCellLabel), 9, 0, 1, 1);
	gtk_widget_set_hexpand (label, true);
	gtk_widget_set_halign (label, GTK_ALIGN_END);
	gtk_widget_set_hexpand (CAN_LABEL_FLOAT_STATIC_TO_WIDGET (&page->maxCellLabel), true);
	gtk_widget_set_halign (CAN_LABEL_FLOAT_STATIC_TO_WIDGET (&page->maxCellLabel), GTK_ALIGN_START);

	label = gtk_label_new ("Avg Cell: ");
	page->avgCellLabel = (canLabelFloatStatic_t)
	{
		.formatValue	= "%.3f%s",
		.formatInvalid	= "%s%s"
	};
	canLabelFloatStaticInit (&page->avgCellLabel);
	gtkLabelSetFont (GTK_LABEL (label), STATUS_FONT);
	gtkLabelSetFont (CAN_LABEL_FLOAT_STATIC_TO_LABEL (&page->avgCellLabel), STATUS_FONT);
	gtk_grid_attach (GTK_GRID (statusPanel), label, 10, 0, 1, 1);
	gtk_grid_attach (GTK_GRID (statusPanel), CAN_LABEL_FLOAT_STATIC_TO_WIDGET (&page->avgCellLabel), 11, 0, 1, 1);
	gtk_widget_set_hexpand (label, true);
	gtk_widget_set_halign (label, GTK_ALIGN_END);
	gtk_widget_set_hexpand (CAN_LABEL_FLOAT_STATIC_TO_WIDGET (&page->avgCellLabel), true);
	gtk_widget_set_halign (CAN_LABEL_FLOAT_STATIC_TO_WIDGET (&page->avgCellLabel), GTK_ALIGN_START);

	label = gtk_label_new ("Min Temp: ");
	page->minTempLabel = (canLabelFloatStatic_t)
	{
		.formatValue	= "%.1f%s",
		.formatInvalid	= "%s%s"
	};
	canLabelFloatStaticInit (&page->minTempLabel);
	gtkLabelSetFont (GTK_LABEL (label), STATUS_FONT);
	gtkLabelSetFont (CAN_LABEL_FLOAT_STATIC_TO_LABEL (&page->minTempLabel), STATUS_FONT);
	gtk_grid_attach (GTK_GRID (statusPanel), label, 0, 1, 1, 1);
	gtk_grid_attach (GTK_GRID (statusPanel), CAN_LABEL_FLOAT_STATIC_TO_WIDGET (&page->minTempLabel), 1, 1, 1, 1);
	gtk_widget_set_hexpand (label, true);
	gtk_widget_set_halign (label, GTK_ALIGN_END);
	gtk_widget_set_hexpand (CAN_LABEL_FLOAT_STATIC_TO_WIDGET (&page->minTempLabel), true);
	gtk_widget_set_halign (CAN_LABEL_FLOAT_STATIC_TO_WIDGET (&page->minTempLabel), GTK_ALIGN_START);

	label = gtk_label_new ("Max Temp: ");
	page->maxTempLabel = (canLabelFloatStatic_t)
	{
		.formatValue	= "%.1f%s",
		.formatInvalid	= "%s%s"
	};
	canLabelFloatStaticInit (&page->maxTempLabel);
	gtkLabelSetFont (GTK_LABEL (label), STATUS_FONT);
	gtkLabelSetFont (CAN_LABEL_FLOAT_STATIC_TO_LABEL (&page->maxTempLabel), STATUS_FONT);
	gtk_grid_attach (GTK_GRID (statusPanel), label, 2, 1, 1, 1);
	gtk_grid_attach (GTK_GRID (statusPanel), CAN_LABEL_FLOAT_STATIC_TO_WIDGET (&page->maxTempLabel), 3, 1, 1, 1);
	gtk_widget_set_hexpand (label, true);
	gtk_widget_set_halign (label, GTK_ALIGN_END);
	gtk_widget_set_hexpand (CAN_LABEL_FLOAT_STATIC_TO_WIDGET (&page->maxTempLabel), true);
	gtk_widget_set_halign (CAN_LABEL_FLOAT_STATIC_TO_WIDGET (&page->maxTempLabel), GTK_ALIGN_START);

	label = gtk_label_new ("Avg Temp: ");
	page->avgTempLabel = (canLabelFloatStatic_t)
	{
		.formatValue	= "%.1f%s",
		.formatInvalid	= "%s%s"
	};
	canLabelFloatStaticInit (&page->avgTempLabel);
	gtkLabelSetFont (GTK_LABEL (label), STATUS_FONT);
	gtkLabelSetFont (CAN_LABEL_FLOAT_STATIC_TO_LABEL (&page->avgTempLabel), STATUS_FONT);
	gtk_grid_attach (GTK_GRID (statusPanel), label, 4, 1, 1, 1);
	gtk_grid_attach (GTK_GRID (statusPanel), CAN_LABEL_FLOAT_STATIC_TO_WIDGET (&page->avgTempLabel), 5, 1, 1, 1);
	gtk_widget_set_hexpand (label, true);
	gtk_widget_set_halign (label, GTK_ALIGN_END);
	gtk_widget_set_hexpand (CAN_LABEL_FLOAT_STATIC_TO_WIDGET (&page->avgTempLabel), true);
	gtk_widget_set_halign (CAN_LABEL_FLOAT_STATIC_TO_WIDGET (&page->avgTempLabel), GTK_ALIGN_START);

	label = gtk_label_new ("Max Delta: ");
	page->maxDeltaLabel = (canLabelFloatStatic_t)
	{
		.formatValue	= "%.3f%s",
		.formatInvalid	= "%s%s"
	};
	canLabelFloatStaticInit (&page->maxDeltaLabel);
	gtkLabelSetFont (GTK_LABEL (label), STATUS_FONT);
	gtkLabelSetFont (CAN_LABEL_FLOAT_STATIC_TO_LABEL (&page->maxDeltaLabel), STATUS_FONT);
	gtk_grid_attach (GTK_GRID (statusPanel), label, 6, 1, 1, 1);
	gtk_grid_attach (GTK_GRID (statusPanel), CAN_LABEL_FLOAT_STATIC_TO_WIDGET (&page->maxDeltaLabel), 7, 1, 1, 1);
	gtk_widget_set_hexpand (label, true);
	gtk_widget_set_halign (label, GTK_ALIGN_END);
	gtk_widget_set_hexpand (CAN_LABEL_FLOAT_STATIC_TO_WIDGET (&page->maxDeltaLabel), true);
	gtk_widget_set_halign (CAN_LABEL_FLOAT_STATIC_TO_WIDGET (&page->maxDeltaLabel), GTK_ALIGN_START);

	label = gtk_label_new ("Avg Delta: ");
	page->avgDeltaLabel = (canLabelFloatStatic_t)
	{
		.formatValue	= "%.3f%s",
		.formatInvalid	= "%s%s"
	};
	canLabelFloatStaticInit (&page->avgDeltaLabel);
	gtkLabelSetFont (GTK_LABEL (label), STATUS_FONT);
	gtkLabelSetFont (CAN_LABEL_FLOAT_STATIC_TO_LABEL (&page->avgDeltaLabel), STATUS_FONT);
	gtk_grid_attach (GTK_GRID (statusPanel), label, 8, 1, 1, 1);
	gtk_grid_attach (GTK_GRID (statusPanel), CAN_LABEL_FLOAT_STATIC_TO_WIDGET (&page->avgDeltaLabel), 9, 1, 1, 1);
	gtk_widget_set_hexpand (label, true);
	gtk_widget_set_halign (label, GTK_ALIGN_END);
	gtk_widget_set_hexpand (CAN_LABEL_FLOAT_STATIC_TO_WIDGET (&page->avgDeltaLabel), true);
	gtk_widget_set_halign (CAN_LABEL_FLOAT_STATIC_TO_WIDGET (&page->avgDeltaLabel), GTK_ALIGN_START);

	label = gtk_label_new ("Cell Voltages:");
	GdkRGBA c = gdkHexToColor ("#7FD1D1");
	gtkLabelSetFont (GTK_LABEL (label), PANEL_TITLE_FONT);
	gtkLabelSetColor (GTK_LABEL (label), &c);
	gtk_widget_set_hexpand (label, true);
	gtk_widget_set_halign (label, GTK_ALIGN_START);
	gtk_grid_attach (GTK_GRID (grid), label, 0, 2, 2, 1);

	bmsBarGraphInit (&page->voltages, bms, &(bmsBarGraphConfig_t)
	{
		.accessor		= bmsGetCellVoltage,
		.offset			= 0,
		.count			= bms->cellCount,
		.barSize		= 3.5f,
		.barSpacing		= 0.5f,
		.length			= 60,
		.min			= bms->minCellVoltage,
		.max			= bms->maxCellVoltage,
		.tickSpacing	= 0.25,
		.tickFormat		= "%.2f V",
		.axisPosition	= 40,
		.axisColor		= gdkHexToColor ("#7FD1D1"),
		.tickColor		= gdkHexToColor ("#406D6D"),
		.barColor		= gdkHexToColor ("#7FD1D1")
	});
	gtk_widget_set_halign (BMS_BAR_GRAPH_TO_WIDGET (&page->voltages), GTK_ALIGN_FILL);
	gtk_widget_set_valign (BMS_BAR_GRAPH_TO_WIDGET (&page->voltages), GTK_ALIGN_FILL);
	gtk_widget_set_hexpand (BMS_BAR_GRAPH_TO_WIDGET (&page->voltages), true);
	gtk_widget_set_vexpand (BMS_BAR_GRAPH_TO_WIDGET (&page->voltages), true);
	gtk_widget_set_margin_top (BMS_BAR_GRAPH_TO_WIDGET (&page->voltages), 5);
	gtk_widget_set_margin_bottom (BMS_BAR_GRAPH_TO_WIDGET (&page->voltages), 5);
	gtk_widget_set_margin_start (BMS_BAR_GRAPH_TO_WIDGET (&page->voltages), 5);
	gtk_widget_set_margin_end (BMS_BAR_GRAPH_TO_WIDGET (&page->voltages), 5);
	gtk_grid_attach (GTK_GRID (grid), BMS_BAR_GRAPH_TO_WIDGET (&page->voltages), 0, 3, 2, 1);

	label = gtk_label_new ("Sense Line Temperatures:");
	c = gdkHexToColor ("#9ee396");
	gtkLabelSetFont (GTK_LABEL (label), PANEL_TITLE_FONT);
	gtkLabelSetColor (GTK_LABEL (label), &c);
	gtk_widget_set_hexpand (label, true);
	gtk_widget_set_halign (label, GTK_ALIGN_START);
	gtk_grid_attach (GTK_GRID (grid), label, 0, 4, 1, 1);

	bmsBarGraphInit (&page->temperatures, bms, &(bmsBarGraphConfig_t)
	{
		.accessor		= bmsGetLogicalTemperature,
		.offset			= 0,
		.count			= bmsGetLogicalTemperatureCount (bms),
		.barSize		= 3.5f,
		.barSpacing		= 0.5f,
		.length			= 60,
		.min			= bms->minTemperature,
		.max			= bms->maxTemperature,
		.tickSpacing	= 10,
		.tickFormat		= "%.0f C",
		.axisPosition	= 40,
		.axisColor		= gdkHexToColor ("#9ee396"),
		.tickColor		= gdkHexToColor ("#4C9044"),
		.barColor		= gdkHexToColor ("#9ee396")
	});
	gtk_widget_set_halign (BMS_BAR_GRAPH_TO_WIDGET (&page->temperatures), GTK_ALIGN_FILL);
	gtk_widget_set_valign (BMS_BAR_GRAPH_TO_WIDGET (&page->temperatures), GTK_ALIGN_FILL);
	gtk_widget_set_hexpand (BMS_BAR_GRAPH_TO_WIDGET (&page->temperatures), true);
	gtk_widget_set_vexpand (BMS_BAR_GRAPH_TO_WIDGET (&page->temperatures), true);
	gtk_widget_set_margin_top (BMS_BAR_GRAPH_TO_WIDGET (&page->temperatures), 5);
	gtk_widget_set_margin_bottom (BMS_BAR_GRAPH_TO_WIDGET (&page->temperatures), 5);
	gtk_widget_set_margin_start (BMS_BAR_GRAPH_TO_WIDGET (&page->temperatures), 5);
	gtk_widget_set_margin_end (BMS_BAR_GRAPH_TO_WIDGET (&page->temperatures), 5);
	gtk_grid_attach (GTK_GRID (grid), BMS_BAR_GRAPH_TO_WIDGET (&page->temperatures), 0, 5, 1, 1);

	label = gtk_label_new ("LTC Temperatures:");
	gtkLabelSetFont (GTK_LABEL (label), PANEL_TITLE_FONT);
	gtkLabelSetColor (GTK_LABEL (label), &c);
	gtk_widget_set_hexpand (label, true);
	gtk_widget_set_halign (label, GTK_ALIGN_START);
	gtk_grid_attach (GTK_GRID (grid), label, 1, 4, 1, 1);

	bmsBarGraphInit (&page->ltcTemperatures, bms, &(bmsBarGraphConfig_t)
	{
		.accessor		= bmsGetLtcTemperature,
		.offset			= 0,
		.count			= bms->ltcCount,
		.barSize		= 3.5f,
		.barSpacing		= 0.5f,
		.length			= 60,
		.min			= bms->minTemperature,
		.max			= bms->maxLtcTemperature,
		.tickSpacing	= 10,
		.tickFormat		= "%.0f C",
		.axisPosition	= 40,
		.axisColor		= gdkHexToColor ("#9ee396"),
		.tickColor		= gdkHexToColor ("#4C9044"),
		.barColor		= gdkHexToColor ("#9ee396")
	});
	gtk_widget_set_halign (BMS_BAR_GRAPH_TO_WIDGET (&page->ltcTemperatures), GTK_ALIGN_FILL);
	gtk_widget_set_valign (BMS_BAR_GRAPH_TO_WIDGET (&page->ltcTemperatures), GTK_ALIGN_FILL);
	gtk_widget_set_hexpand (BMS_BAR_GRAPH_TO_WIDGET (&page->ltcTemperatures), true);
	gtk_widget_set_vexpand (BMS_BAR_GRAPH_TO_WIDGET (&page->ltcTemperatures), true);
	gtk_widget_set_margin_top (BMS_BAR_GRAPH_TO_WIDGET (&page->ltcTemperatures), 5);
	gtk_widget_set_margin_bottom (BMS_BAR_GRAPH_TO_WIDGET (&page->ltcTemperatures), 5);
	gtk_widget_set_margin_start (BMS_BAR_GRAPH_TO_WIDGET (&page->ltcTemperatures), 5);
	gtk_widget_set_margin_end (BMS_BAR_GRAPH_TO_WIDGET (&page->ltcTemperatures), 5);
	gtk_grid_attach (GTK_GRID (grid), BMS_BAR_GRAPH_TO_WIDGET (&page->ltcTemperatures), 1, 5, 1, 1);

	page->buttonPanel = GTK_GRID (gtk_grid_new ());
	gtk_widget_set_margin_bottom (GTK_WIDGET (page->buttonPanel), 10);
	gtk_widget_set_margin_start (GTK_WIDGET (page->buttonPanel), 100);
	gtk_widget_set_margin_end (GTK_WIDGET (page->buttonPanel), 20);
	gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (page->buttonPanel), 0, 6, 2, 1);

	return (page_t*) page;
}

void pageBmsAppendButton (void* page, const char* label, pageButtonCallback_t* callback, void* arg, bool currentPage)
{
	(void) currentPage;
	pageBmsOverview_t* pageBms = page;

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

	gtk_grid_attach (GTK_GRID (pageBms->buttonPanel), STYLIZED_BUTTON_TO_WIDGET (button), pageBms->buttonCount + 1, 0, 1, 1);
	++pageBms->buttonCount;
}

void pageBmsOverviewUpdate (void* page)
{
	pageBmsOverview_t* pageBms = page;

	bmsBarGraphUpdate (&pageBms->voltages);
	bmsBarGraphUpdate (&pageBms->temperatures);
	bmsBarGraphUpdate (&pageBms->ltcTemperatures);

	bmsFaultLabelUpdate (&pageBms->faultLabel);

	float value;
	canDatabaseSignalState_t state = bmsGetPackVoltage (pageBms->bms, &value);
	canLabelFloatStaticUpdate (&pageBms->voltageLabel, value, state, "V");

	state = bmsGetPackCurrent (pageBms->bms, &value);
	canLabelFloatStaticUpdate (&pageBms->currentLabel, value, state, "A");

	state = bmsGetPackPower (pageBms->bms, &value);
	canLabelFloatStaticUpdate (&pageBms->powerLabel, value, state, "W");

	float min;
	float max;
	float avg;
	bool valid = bmsGetCellVoltageStats (pageBms->bms, &min, &max, &avg);
	state = valid ? CAN_DATABASE_VALID : CAN_DATABASE_TIMEOUT;
	canLabelFloatStaticUpdate (&pageBms->minCellLabel, min, state, "V");
	canLabelFloatStaticUpdate (&pageBms->maxCellLabel, max, state, "V");
	canLabelFloatStaticUpdate (&pageBms->avgCellLabel, avg, state, "V");

	valid = bmsGetTemperatureStats (pageBms->bms, &min, &max, &avg);
	state = valid ? CAN_DATABASE_VALID : CAN_DATABASE_TIMEOUT;
	canLabelFloatStaticUpdate (&pageBms->minTempLabel, min, state, "C");
	canLabelFloatStaticUpdate (&pageBms->maxTempLabel, max, state, "C");
	canLabelFloatStaticUpdate (&pageBms->avgTempLabel, avg, state, "C");

	valid = bmsGetCellDeltaStats (pageBms->bms, &max, &avg);
	state = valid ? CAN_DATABASE_VALID : CAN_DATABASE_TIMEOUT;
	canLabelFloatStaticUpdate (&pageBms->maxDeltaLabel, max, state, "V");
	canLabelFloatStaticUpdate (&pageBms->avgDeltaLabel, avg, state, "V");
}
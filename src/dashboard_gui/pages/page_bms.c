// Header
#include "page_bms.h"

// Includes
#include "../stylized_widgets/stylized_button.h"
#include "../gtk_util.h"
#include "cjson/cjson_util.h"
#include "debug.h"

// POSIX
#include <wordexp.h>

#define STATUS_FONT				"Monospace 12px"

static void drawBg (GtkDrawingArea* area, cairo_t* cr, int width, int height, gpointer arg)
{
	pageBms_t* page = arg;
	(void) area;
	(void) arg;

	(void) page;

	cairo_rectangle (cr, 0, 0, width, height);
	cairo_fill (cr);
}

static void styleLoad (pageBmsStyle_t* style, pageStyle_t* baseStyle, cJSON* config)
{
	*style = (pageBmsStyle_t)
	{
		.baseStyle = baseStyle
	};

	if (config == NULL)
		return;

	style->baseStyle = pageStyleLoad (jsonGetObjectV2 (config, "baseStyle"), baseStyle);

	jsonGetString (config, "cellVoltageGraphTitleFont", &style->cellVoltageGraphTitleFont);

	char* cellVoltageGraphTitleColor;
	if (jsonGetString (config, "cellVoltageGraphTitleColor", &cellVoltageGraphTitleColor) == 0)
		style->cellVoltageGraphTitleColor = gdkHexToColor (cellVoltageGraphTitleColor);

	char* cellVoltageGraphBarColor;
	if (jsonGetString (config, "cellVoltageGraphBarColor", &cellVoltageGraphBarColor) == 0)
		style->cellVoltageGraphBarColor = gdkHexToColor (cellVoltageGraphBarColor);

	char* cellVoltageGraphTickColor;
	if (jsonGetString (config, "cellVoltageGraphTickColor", &cellVoltageGraphTickColor) == 0)
		style->cellVoltageGraphTickColor = gdkHexToColor (cellVoltageGraphTickColor);

	char* cellVoltageGraphAxisColor;
	if (jsonGetString (config, "cellVoltageGraphAxisColor", &cellVoltageGraphAxisColor) == 0)
		style->cellVoltageGraphAxisColor = gdkHexToColor (cellVoltageGraphAxisColor);

	jsonGetString (config, "senseLineTemperatureGraphTitleFont", &style->senseLineTemperatureGraphTitleFont);

	char* senseLineTemperatureGraphTitleColor;
	if (jsonGetString (config, "senseLineTemperatureGraphTitleColor", &senseLineTemperatureGraphTitleColor) == 0)
		style->senseLineTemperatureGraphTitleColor = gdkHexToColor (senseLineTemperatureGraphTitleColor);

	char* senseLineTemperatureGraphBarColor;
	if (jsonGetString (config, "senseLineTemperatureGraphBarColor", &senseLineTemperatureGraphBarColor) == 0)
		style->senseLineTemperatureGraphBarColor = gdkHexToColor (senseLineTemperatureGraphBarColor);

	char* senseLineTemperatureGraphTickColor;
	if (jsonGetString (config, "senseLineTemperatureGraphTickColor", &senseLineTemperatureGraphTickColor) == 0)
		style->senseLineTemperatureGraphTickColor = gdkHexToColor (senseLineTemperatureGraphTickColor);

	char* senseLineTemperatureGraphAxisColor;
	if (jsonGetString (config, "senseLineTemperatureGraphAxisColor", &senseLineTemperatureGraphAxisColor) == 0)
		style->senseLineTemperatureGraphAxisColor = gdkHexToColor (senseLineTemperatureGraphAxisColor);

	jsonGetString (config, "ltcTemperatureGraphTitleFont", &style->ltcTemperatureGraphTitleFont);

	char* ltcTemperatureGraphTitleColor;
	if (jsonGetString (config, "ltcTemperatureGraphTitleColor", &ltcTemperatureGraphTitleColor) == 0)
		style->ltcTemperatureGraphTitleColor = gdkHexToColor (ltcTemperatureGraphTitleColor);

	char* ltcTemperatureGraphBarColor;
	if (jsonGetString (config, "ltcTemperatureGraphBarColor", &ltcTemperatureGraphBarColor) == 0)
		style->ltcTemperatureGraphBarColor = gdkHexToColor (ltcTemperatureGraphBarColor);

	char* ltcTemperatureGraphTickColor;
	if (jsonGetString (config, "ltcTemperatureGraphTickColor", &ltcTemperatureGraphTickColor) == 0)
		style->ltcTemperatureGraphTickColor = gdkHexToColor (ltcTemperatureGraphTickColor);

	char* ltcTemperatureGraphAxisColor;
	if (jsonGetString (config, "ltcTemperatureGraphAxisColor", &ltcTemperatureGraphAxisColor) == 0)
		style->ltcTemperatureGraphAxisColor = gdkHexToColor (ltcTemperatureGraphAxisColor);
}

static void appendButton (void* pageArg, const char* label, pageButtonCallback_t* callback, void* arg, bool currentPage)
{
	pageBms_t* page = pageArg;

	stylizedButton_t* button = stylizedButtonInit (callback, arg, &(stylizedButtonConfig_t)
	{
		.width				= 100,
		.height				= page->style.baseStyle->buttonHeight,
		.label				= label,
		.borderThickness	= page->style.baseStyle->borderThickness,
		.backgroundColor	= page->style.baseStyle->backgroundColor,
		.borderColor		= page->style.baseStyle->borderColor,
		.selectedColor		= page->style.baseStyle->fontColor,
		.indicatorColor		= currentPage ?
			page->style.baseStyle->indicatorActiveColor : page->style.baseStyle->indicatorInactiveColor
	});
	gtkLabelSetFont (STYLIZED_BUTTON_TO_LABEL (button), page->style.baseStyle->buttonFont);
	gtk_widget_set_margin_top (STYLIZED_BUTTON_TO_WIDGET (button), 8);
	gtk_widget_set_margin_start (STYLIZED_BUTTON_TO_WIDGET (button), 4);
	gtk_widget_set_margin_end (STYLIZED_BUTTON_TO_WIDGET (button), 4);
	gtk_widget_set_hexpand (STYLIZED_BUTTON_TO_WIDGET (button), true);
	gtk_grid_attach (GTK_GRID (page->buttonPanel), STYLIZED_BUTTON_TO_WIDGET (button), page->buttonCount, 0, 1, 1);
	++page->buttonCount;
}

static void update (void* pageArg)
{
	pageBms_t* page = pageArg;

	bmsBarGraphUpdate (&page->voltages);
	bmsBarGraphUpdate (&page->temperatures);
	bmsBarGraphUpdate (&page->ltcTemperatures);

	bmsFaultLabelUpdate (&page->faultLabel);

	float value;
	canDatabaseSignalState_t state = bmsGetPackVoltage (&page->bms, &value);
	canLabelFloatStaticUpdate (&page->voltageLabel, value, state, "V");

	state = bmsGetPackCurrent (&page->bms, &value);
	canLabelFloatStaticUpdate (&page->currentLabel, value, state, "A");

	state = bmsGetPackPower (&page->bms, &value);
	canLabelFloatStaticUpdate (&page->powerLabel, value, state, "W");

	float min;
	float max;
	float avg;
	bool valid = bmsGetCellVoltageStats (&page->bms, &min, &max, &avg);
	state = valid ? CAN_DATABASE_VALID : CAN_DATABASE_TIMEOUT;
	canLabelFloatStaticUpdate (&page->minCellLabel, min, state, "V");
	canLabelFloatStaticUpdate (&page->maxCellLabel, max, state, "V");
	canLabelFloatStaticUpdate (&page->avgCellLabel, avg, state, "V");

	valid = bmsGetTemperatureStats (&page->bms, &min, &max, &avg);
	state = valid ? CAN_DATABASE_VALID : CAN_DATABASE_TIMEOUT;
	canLabelFloatStaticUpdate (&page->minTempLabel, min, state, "C");
	canLabelFloatStaticUpdate (&page->maxTempLabel, max, state, "C");
	canLabelFloatStaticUpdate (&page->avgTempLabel, avg, state, "C");

	valid = bmsGetCellDeltaStats (&page->bms, &max, &avg);
	state = valid ? CAN_DATABASE_VALID : CAN_DATABASE_TIMEOUT;
	canLabelFloatStaticUpdate (&page->maxDeltaLabel, max, state, "V");
	canLabelFloatStaticUpdate (&page->avgDeltaLabel, avg, state, "V");
}

page_t* pageBmsLoad (cJSON* config, canDatabase_t* database, pageStyle_t* style)
{
	if (config == NULL)
		return NULL;

	char* pageType;
	if (jsonGetString (config, "type", &pageType) != 0)
		return NULL;

	if (strcmp (pageType, "pageBms_t") != 0)
		return NULL;

	char* pageName;
	if (jsonGetString (config, "name", &pageName) != 0)
		return NULL;

	pageBms_t* page = malloc (sizeof (pageBms_t));
	if (page == NULL)
		return NULL;

	// Setup the VMT
	page->vmt = (pageVmt_t)
	{
		.update			= update,
		.appendButton	= appendButton,
		.widget			= gtk_overlay_new (),
		.name			= pageName,
		.parent			= NULL
	};

	char* bmsConfigPath;
	if (jsonGetString (config, "bmsConfig", &bmsConfigPath) != 0)
	{
		debugPrintf ("Warning, BMS config JSON not specified.\n");
		return NULL;
	}
	// TODO(Barach): Wordexp
	cJSON* bmsConfig = jsonLoad (bmsConfigPath);
	if (bmsConfig == NULL)
	{
		debugPrintf ("Warning, failed to load BMS config JSON.\n");
		return NULL;
	}

	if (bmsInit (&page->bms, bmsConfig, database) != 0)
	{
		debugPrintf ("Warning, failed to initialize BMS interface.\n");
		return NULL;
	}

	cJSON* styleConfig = jsonGetObjectV2 (config, "style");
	styleLoad (&page->style, style, styleConfig);

	page->buttonCount = 0;

	GtkWidget* bg = gtk_drawing_area_new ();
	gtk_drawing_area_set_draw_func (GTK_DRAWING_AREA (bg), drawBg, page, NULL);
	gtk_overlay_set_child (GTK_OVERLAY (page->vmt.widget), bg);

	GtkWidget* grid = gtk_grid_new ();
	gtk_overlay_add_overlay (GTK_OVERLAY (page->vmt.widget), grid);
	gtk_overlay_set_measure_overlay (GTK_OVERLAY (page->vmt.widget), grid, true);

	GtkWidget* statusTitle = gtk_grid_new ();
	gtk_grid_attach (GTK_GRID (grid), statusTitle, 0, 0, 2, 1);

	GtkWidget* label = gtk_label_new ("BMS Status: ");
	gtk_grid_attach (GTK_GRID (statusTitle), label, 0, 0, 1, 1);
	gtkLabelSetFont (GTK_LABEL (label), STATUS_FONT);

	bmsFaultLabelInit (&page->faultLabel, &page->bms);
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
	gtkLabelSetFont (GTK_LABEL (label), page->style.cellVoltageGraphTitleFont);
	gtkLabelSetColor (GTK_LABEL (label), &page->style.cellVoltageGraphTitleColor);
	gtk_widget_set_hexpand (label, true);
	gtk_widget_set_halign (label, GTK_ALIGN_START);
	gtk_grid_attach (GTK_GRID (grid), label, 0, 2, 2, 1);

	bmsBarGraphInit (&page->voltages, &page->bms, &(bmsBarGraphConfig_t)
	{
		.accessor		= bmsGetCellVoltage,
		.offset			= 0,
		.count			= page->bms.cellCount,
		.barSize		= 3.5f,
		.barSpacing		= 0.5f,
		.length			= 60,
		.min			= page->bms.minCellVoltage,
		.max			= page->bms.maxCellVoltage,
		.tickSpacing	= 0.25,
		.tickFormat		= "%.2f V",
		.axisPosition	= 40,
		.axisColor		= page->style.cellVoltageGraphAxisColor,
		.tickColor		= page->style.cellVoltageGraphTickColor,
		.barColor		= page->style.cellVoltageGraphBarColor
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

	label = gtk_label_new ("Cell Sense Line Temperatures:");
	gtkLabelSetFont (GTK_LABEL (label), page->style.senseLineTemperatureGraphTitleFont);
	gtkLabelSetColor (GTK_LABEL (label), &page->style.senseLineTemperatureGraphTitleColor);
	gtk_widget_set_hexpand (label, true);
	gtk_widget_set_halign (label, GTK_ALIGN_START);
	gtk_grid_attach (GTK_GRID (grid), label, 0, 4, 1, 1);

	bmsBarGraphInit (&page->temperatures, &page->bms, &(bmsBarGraphConfig_t)
	{
		.accessor		= bmsGetLogicalTemperature,
		.offset			= 0,
		.count			= bmsGetLogicalTemperatureCount (&page->bms),
		.barSize		= 3.5f,
		.barSpacing		= 0.5f,
		.length			= 60,
		.min			= page->bms.minTemperature,
		.max			= page->bms.maxTemperature,
		.tickSpacing	= 10,
		.tickFormat		= "%.0f C",
		.axisPosition	= 40,
		.axisColor		= page->style.senseLineTemperatureGraphAxisColor,
		.tickColor		= page->style.senseLineTemperatureGraphTickColor,
		.barColor		= page->style.senseLineTemperatureGraphBarColor
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

	label = gtk_label_new ("Sense Board IC Temperatures:");
	gtkLabelSetFont (GTK_LABEL (label), page->style.ltcTemperatureGraphTitleFont);
	gtkLabelSetColor (GTK_LABEL (label), &page->style.ltcTemperatureGraphTitleColor);
	gtk_widget_set_hexpand (label, true);
	gtk_widget_set_halign (label, GTK_ALIGN_START);
	gtk_grid_attach (GTK_GRID (grid), label, 1, 4, 1, 1);

	bmsBarGraphInit (&page->ltcTemperatures, &page->bms, &(bmsBarGraphConfig_t)
	{
		.accessor		= bmsGetLtcTemperature,
		.offset			= 0,
		.count			= page->bms.ltcCount,
		.barSize		= 3.5f,
		.barSpacing		= 0.5f,
		.length			= 60,
		.min			= page->bms.minTemperature,
		.max			= page->bms.maxLtcTemperature,
		.tickSpacing	= 10,
		.tickFormat		= "%.0f C",
		.axisPosition	= 40,
		.axisColor		= page->style.ltcTemperatureGraphAxisColor,
		.tickColor		= page->style.ltcTemperatureGraphTickColor,
		.barColor		= page->style.ltcTemperatureGraphBarColor
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
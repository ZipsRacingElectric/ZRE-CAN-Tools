// Header
#include "page_bms.h"

// Includes
#include "../stylized_widgets/stylized_button.h"
#include "../gtk_util.h"
#include "can_database/can_database_stdio.h"
#include "cjson/cjson_util.h"
#include "debug.h"

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

	char* terminalBackgroundColor;
	if (jsonGetString (config, "terminalBackgroundColor", &terminalBackgroundColor) == 0)
		style->terminalBackgroundColor = gdkHexToColor (terminalBackgroundColor);

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

static void appendButton (void* pageArg, const char* label, pageButtonCallback_t* callback, void* arg, bool currentPage, pageStyle_t* style)
{
	pageBms_t* page = pageArg;

	stylizedButton_t* button = stylizedButtonInit (callback, arg, &(stylizedButtonConfig_t)
	{
		.width				= 100,
		.height				= style->buttonHeight,
		.label				= label,
		.borderThickness	= style->borderThickness,
		.backgroundColor	= style->backgroundColor,
		.borderColor		= style->borderColor,
		.selectedColor		= style->fontColor,
		.indicatorColor		= currentPage ?
			page->style.baseStyle->indicatorActiveColor : page->style.baseStyle->indicatorInactiveColor
	});
	gtkLabelSetFont (STYLIZED_BUTTON_TO_LABEL (button), style->buttonFont);
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

	size_t bufferSize;
	char* buffer = stylizedTerminalGetBuffer (page->term, &bufferSize);
	if (buffer == NULL)
		return;

	if (stylizedTerminalSnprintf (page->term, &buffer, &bufferSize, "- BMS Status: %s ", bmsGetFaultState (&page->bms)) != 0)
		return;

	for (size_t index = 0; index < bufferSize - 1; ++index)
		buffer [index] = '-';
	buffer [bufferSize - 1] = '\0';

	if (stylizedTerminalPrintNewline (page->term, &buffer, &bufferSize) != 0)
		return;

	float voltage, current, power;
	canDatabaseSignalState_t voltageState = bmsGetPackVoltage (&page->bms, &voltage);
	canDatabaseSignalState_t currentState = bmsGetPackCurrent (&page->bms, &current);
	canDatabaseSignalState_t powerState = bmsGetPackPower (&page->bms, &power);

	float cellMin, cellMax, cellAvg;
	bool valid = bmsGetCellVoltageStats (&page->bms, &cellMin, &cellMax, &cellAvg);
	canDatabaseSignalState_t cellStatsState = valid ? CAN_DATABASE_VALID : CAN_DATABASE_TIMEOUT;

	float tempMin, tempMax, tempAvg;
	valid = bmsGetTemperatureStats (&page->bms, &tempMin, &tempMax, &tempAvg);
	canDatabaseSignalState_t tempStatsState = valid ? CAN_DATABASE_VALID : CAN_DATABASE_TIMEOUT;

	float deltaMax, deltaAvg;
	valid = bmsGetCellDeltaStats (&page->bms, &deltaMax, &deltaAvg);
	canDatabaseSignalState_t deltaStatsState = valid ? CAN_DATABASE_VALID : CAN_DATABASE_TIMEOUT;

	if (stylizedTerminalSnprintCallback (page->term, buffer, bufferSize,
		snprintCanDatabaseFloatStatic, "  Voltage: %7.1f%s", "  Voltage: %7s%s", voltage, voltageState, "V") != 0)
	{
		return;
	}

	if (stylizedTerminalSnprintCallback (page->term, buffer, bufferSize,
		snprintCanDatabaseFloatStatic, "  |  Min Cell: %6.3f%s", "  |  Min Cell: %6s%s", cellMin, cellStatsState, "V") != 0)
	{
		return;
	}

	if (stylizedTerminalSnprintCallback (page->term, buffer, bufferSize,
		snprintCanDatabaseFloatStatic, "  |  Min Temp: %6.2f%s", "  |  Min Temp: %6s%s", tempMin, tempStatsState, "C") != 0)
	{
		return;
	}

	if (stylizedTerminalSnprintCallback (page->term, buffer, bufferSize,
		snprintCanDatabaseFloatStatic, "  |  Max Delta: %5.3f%s", "  |  Max Delta: %5s%s", deltaMax, deltaStatsState, "V") != 0)
	{
		return;
	}

	if (stylizedTerminalPrintNewline (page->term, &buffer, &bufferSize) != 0)
		return;


	if (stylizedTerminalSnprintCallback (page->term, buffer, bufferSize,
		snprintCanDatabaseFloatStatic, "  Current: %7.1f%s", "  Current: %7s%s", current, currentState, "A") != 0)
	{
		return;
	}

	if (stylizedTerminalSnprintCallback (page->term, buffer, bufferSize,
		snprintCanDatabaseFloatStatic, "  |  Max Cell: %6.3f%s", "  |  Max Cell: %6s%s", cellMax, cellStatsState, "V") != 0)
	{
		return;
	}

	if (stylizedTerminalSnprintCallback (page->term, buffer, bufferSize,
		snprintCanDatabaseFloatStatic, "  |  Max Temp: %6.2f%s", "  |  Max Temp: %6s%s", tempMax, tempStatsState, "C") != 0)
	{
		return;
	}

	if (stylizedTerminalSnprintCallback (page->term, buffer, bufferSize,
		snprintCanDatabaseFloatStatic, "  |  Avg Delta: %5.3f%s", "  |  Avg Delta: %5s%s", deltaAvg, deltaStatsState, "V") != 0)
	{
		return;
	}

	if (stylizedTerminalPrintNewline (page->term, &buffer, &bufferSize) != 0)
		return;

	if (stylizedTerminalSnprintCallback (page->term, buffer, bufferSize,
		snprintCanDatabaseFloatStatic, "  Power: %9.1f%s", "  Power: %9s%s", power, powerState, "W") != 0)
	{
		return;
	}

	if (stylizedTerminalSnprintCallback (page->term, buffer, bufferSize,
		snprintCanDatabaseFloatStatic, "  |  Avg Cell: %6.3f%s", "  |  Avg Cell: %6s%s", cellAvg, cellStatsState, "V") != 0)
	{
		return;
	}

	if (stylizedTerminalSnprintCallback (page->term, buffer, bufferSize,
		snprintCanDatabaseFloatStatic, "  |  Avg Temp: %6.2f%s  |", "  |  Avg Temp: %6s%s  |", tempAvg, tempStatsState, "C") != 0)
	{
		return;
	}

	if (stylizedTerminalPrintNewline (page->term, &buffer, &bufferSize) != 0)
		return;

	for (size_t index = 0; index < bufferSize - 1; ++index)
		buffer [index] = '-';
	buffer [bufferSize - 1] = '\0';

	if (stylizedTerminalPrintNewline (page->term, &buffer, &bufferSize) != 0)
		return;

	stylizedTerminalWriteBuffer (page->term);
}

page_t* pageBmsLoad (cJSON* config, canDatabase_t* database, pageStyle_t* style)
{
	if (config == NULL)
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
	cJSON* bmsConfig = jsonLoadPath (bmsConfigPath);
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

	page->term = stylizedTerminalInit (&(stylizedTerminalConfig_t)
	{
		.lineLengthMax		= 128,
		.lineCountMax		= 8,
		.fontSize			= 14,
		.fontSpacing		= 2,
		.scrollEnabled		= false,
		.backgroundColor	= page->style.terminalBackgroundColor,
		.fontColor			= page->style.baseStyle->fontColor,
	});
	gtk_widget_set_size_request (STYLIZED_TERMINAL_TO_WIDGET (page->term), 50, 80);
	gtk_grid_attach (GTK_GRID (grid), STYLIZED_TERMINAL_TO_WIDGET (page->term), 0, 0, 2, 1);

	GtkWidget* label = gtk_label_new ("Cell Voltages:");
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
	gtk_widget_set_halign (BMS_BAR_GRAPH_TO_WIDGET (&page->temperatures), GTK_ALIGN_FILL);
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
	gtk_widget_set_halign (BMS_BAR_GRAPH_TO_WIDGET (&page->ltcTemperatures), GTK_ALIGN_FILL);
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
// Header
#include "page_bms.h"

// Includes
#include "../bms_widgets/cell_voltage_bar_graph.h"
#include "../bms_widgets/sense_line_temperature_graph.h"
#include "../bms_widgets/ltc_temperature_graph.h"
#include "../stylized_widgets/stylized_button.h"
#include "../gtk_util.h"
#include "can_database/can_database_stdio.h"
#include "cjson/cjson_util.h"
#include "debug.h"

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

	char* color;
	if (jsonGetString (config, "terminalBackgroundColor", &color) == 0)
		style->terminalBackgroundColor = gdkHexToColor (color);

	jsonGetString (config, "cellVoltageGraphTitleFont", &style->cellVoltageGraphTitleFont);

	if (jsonGetString (config, "cellVoltageGraphTitleColor", &color) == 0)
		style->cellVoltageGraphTitleColor = gdkHexToColor (color);
	if (jsonGetString (config, "cellVoltageGraphValidBarColor", &color) == 0)
		style->cellVoltageGraphValidBarColor = gdkHexToColor (color);
	if (jsonGetString (config, "cellVoltageGraphInvalidBarColor", &color) == 0)
		style->cellVoltageGraphInvalidBarColor = gdkHexToColor (color);
	if (jsonGetString (config, "cellVoltageGraphBalancingBarColor", &color) == 0)
		style->cellVoltageGraphBalancingBarColor = gdkHexToColor (color);
	if (jsonGetString (config, "cellVoltageGraphTickColor", &color) == 0)
		style->cellVoltageGraphTickColor = gdkHexToColor (color);
	if (jsonGetString (config, "cellVoltageGraphTickColor2", &color) == 0)
		style->cellVoltageGraphTickColor2 = gdkHexToColor (color);
	if (jsonGetString (config, "cellVoltageGraphAxisColor", &color) == 0)
		style->cellVoltageGraphAxisColor = gdkHexToColor (color);

	jsonGetString (config, "senseLineTemperatureGraphTitleFont", &style->senseLineTemperatureGraphTitleFont);

	if (jsonGetString (config, "senseLineTemperatureGraphTitleColor", &color) == 0)
		style->senseLineTemperatureGraphTitleColor = gdkHexToColor (color);
	if (jsonGetString (config, "senseLineTemperatureGraphValidBarColor", &color) == 0)
		style->senseLineTemperatureGraphValidBarColor = gdkHexToColor (color);
	if (jsonGetString (config, "senseLineTemperatureGraphInvalidBarColor", &color) == 0)
		style->senseLineTemperatureGraphInvalidBarColor = gdkHexToColor (color);
	if (jsonGetString (config, "senseLineTemperatureGraphTickColor", &color) == 0)
		style->senseLineTemperatureGraphTickColor = gdkHexToColor (color);
	if (jsonGetString (config, "senseLineTemperatureGraphTickColor2", &color) == 0)
		style->senseLineTemperatureGraphTickColor2 = gdkHexToColor (color);
	if (jsonGetString (config, "senseLineTemperatureGraphAxisColor", &color) == 0)
		style->senseLineTemperatureGraphAxisColor = gdkHexToColor (color);

	jsonGetString (config, "ltcTemperatureGraphTitleFont", &style->ltcTemperatureGraphTitleFont);

	if (jsonGetString (config, "ltcTemperatureGraphTitleColor", &color) == 0)
		style->ltcTemperatureGraphTitleColor = gdkHexToColor (color);
	if (jsonGetString (config, "ltcTemperatureGraphValidBarColor", &color) == 0)
		style->ltcTemperatureGraphValidBarColor = gdkHexToColor (color);
	if (jsonGetString (config, "ltcTemperatureGraphInvalidBarColor", &color) == 0)
		style->ltcTemperatureGraphInvalidBarColor = gdkHexToColor (color);
	if (jsonGetString (config, "ltcTemperatureGraphTickColor", &color) == 0)
		style->ltcTemperatureGraphTickColor = gdkHexToColor (color);
	if (jsonGetString (config, "ltcTemperatureGraphTickColor2", &color) == 0)
		style->ltcTemperatureGraphTickColor2 = gdkHexToColor (color);
	if (jsonGetString (config, "ltcTemperatureGraphAxisColor", &color) == 0)
		style->ltcTemperatureGraphAxisColor = gdkHexToColor (color);
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
			page->style.baseStyle->buttonActiveColor : page->style.baseStyle->buttonInactiveColor,
		.useIndicator		= true
	});
	if (style->buttonFont != NULL)
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

	stylizedBarGraphUpdate (&page->voltages);
	stylizedBarGraphUpdate (&page->temperatures);
	stylizedBarGraphUpdate (&page->ltcTemperatures);

	size_t bufferSize;
	char* buffer = stylizedTerminalGetBuffer (page->term, &bufferSize);
	if (buffer == NULL)
		return;

	char* status = faultSignalsGetString (&page->faults);
	if (stylizedTerminalSnprintf (page->term, &buffer, &bufferSize, "- BMS Status: %s ", status) != 0)
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
		return;

	if (stylizedTerminalSnprintCallback (page->term, buffer, bufferSize,
		snprintCanDatabaseFloatStatic, "  |  Min Cell: %6.3f%s", "  |  Min Cell: %6s%s", cellMin, cellStatsState, "V") != 0)
		return;

	if (stylizedTerminalSnprintCallback (page->term, buffer, bufferSize,
		snprintCanDatabaseFloatStatic, "  |  Min Temp: %6.2f%s", "  |  Min Temp: %6s%s", tempMin, tempStatsState, "C") != 0)
		return;

	if (stylizedTerminalSnprintCallback (page->term, buffer, bufferSize,
		snprintCanDatabaseFloatStatic, "  |  Max Delta: %5.3f%s", "  |  Max Delta: %5s%s", deltaMax, deltaStatsState, "V") != 0)
		return;

	if (stylizedTerminalPrintNewline (page->term, &buffer, &bufferSize) != 0)
		return;


	if (stylizedTerminalSnprintCallback (page->term, buffer, bufferSize,
		snprintCanDatabaseFloatStatic, "  Current: %7.1f%s", "  Current: %7s%s", current, currentState, "A") != 0)
		return;

	if (stylizedTerminalSnprintCallback (page->term, buffer, bufferSize,
		snprintCanDatabaseFloatStatic, "  |  Max Cell: %6.3f%s", "  |  Max Cell: %6s%s", cellMax, cellStatsState, "V") != 0)
		return;

	if (stylizedTerminalSnprintCallback (page->term, buffer, bufferSize,
		snprintCanDatabaseFloatStatic, "  |  Max Temp: %6.2f%s", "  |  Max Temp: %6s%s", tempMax, tempStatsState, "C") != 0)
		return;

	if (stylizedTerminalSnprintCallback (page->term, buffer, bufferSize,
		snprintCanDatabaseFloatStatic, "  |  Avg Delta: %5.3f%s", "  |  Avg Delta: %5s%s", deltaAvg, deltaStatsState, "V") != 0)
		return;

	if (stylizedTerminalPrintNewline (page->term, &buffer, &bufferSize) != 0)
		return;

	if (stylizedTerminalSnprintCallback (page->term, buffer, bufferSize,
		snprintCanDatabaseFloatStatic, "  Power: %9.1f%s", "  Power: %9s%s", power, powerState, "W") != 0)
		return;

	if (stylizedTerminalSnprintCallback (page->term, buffer, bufferSize,
		snprintCanDatabaseFloatStatic, "  |  Avg Cell: %6.3f%s", "  |  Avg Cell: %6s%s", cellAvg, cellStatsState, "V") != 0)
		return;

	if (stylizedTerminalSnprintCallback (page->term, buffer, bufferSize,
		snprintCanDatabaseFloatStatic, "  |  Avg Temp: %6.2f%s  |", "  |  Avg Temp: %6s%s  |", tempAvg, tempStatsState, "C") != 0)
		return;

	if (stylizedTerminalPrintNewline (page->term, &buffer, &bufferSize) != 0)
		return;

	for (size_t index = 0; index < bufferSize - 1; ++index)
		buffer [index] = '-';
	buffer [bufferSize - 1] = '\0';

	if (stylizedTerminalPrintNewline (page->term, &buffer, &bufferSize) != 0)
		return;

	stylizedTerminalWriteBuffer (page->term);
}

page_t* pageBmsLoad (cJSON* config, canDatabase_t* databases, size_t databaseCount, pageStyle_t* style)
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

	unsigned deviceIndex;
	if (jsonGetUnsigned (config, "deviceIndex", &deviceIndex) != 0)
	{
		debugPrintf ("Warning, BMS page is missing device index.\n");
		return NULL;
	}

	if (deviceIndex >= databaseCount)
	{
		debugPrintf ("Warning, BMS page specifies invalid device index %u. Device count is %lu.\n",
			deviceIndex, (long unsigned) databaseCount);
		return NULL;
	}

	if (bmsInit (&page->bms, bmsConfig, &databases [deviceIndex]) != 0)
	{
		debugPrintf ("Warning, failed to initialize BMS interface.\n");
		return NULL;
	}

	if (faultSignalsLoad (&page->faults, bmsConfig, &databases [deviceIndex]) != 0)
	{
		debugPrintf ("Warning, failed to load BMS fault array.\n");
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

	stylizedBarGraphInit (&page->voltages, &(stylizedBarGraphConfig_t)
	{
		.accessor			= bmsCellVoltageBarGraphAccessor,
		.foregroundDraw		= bmsCellVoltageBarGraphDrawForeground,
		.offset				= 0,
		.count				= page->bms.cellCount,
		.barSize			= 3.5f,
		.barSpacing			= 0.5f,
		.length				= 60,
		.min				= page->bms.minCellVoltage,
		.max				= page->bms.maxCellVoltage,
		.tickSpacing		= 0.25,
		.tickFormat			= "%.2f V",
		.axisPosition		= 40,
		.axisColor			= page->style.cellVoltageGraphAxisColor,
		.tickColor			= page->style.cellVoltageGraphTickColor,
		.tickColor2			= page->style.cellVoltageGraphTickColor2,
		.validBarColor		= page->style.cellVoltageGraphValidBarColor,
		.invalidBarColor	= page->style.cellVoltageGraphInvalidBarColor,
		.alternateBarColor	= page->style.cellVoltageGraphBalancingBarColor
	}, &page->bms);
	gtk_widget_set_halign (STYLIZED_BAR_GRAPH_TO_WIDGET (&page->voltages), GTK_ALIGN_FILL);
	gtk_widget_set_valign (STYLIZED_BAR_GRAPH_TO_WIDGET (&page->voltages), GTK_ALIGN_FILL);
	gtk_widget_set_hexpand (STYLIZED_BAR_GRAPH_TO_WIDGET (&page->voltages), true);
	gtk_widget_set_vexpand (STYLIZED_BAR_GRAPH_TO_WIDGET (&page->voltages), true);
	gtk_widget_set_margin_top (STYLIZED_BAR_GRAPH_TO_WIDGET (&page->voltages), 5);
	gtk_widget_set_margin_bottom (STYLIZED_BAR_GRAPH_TO_WIDGET (&page->voltages), 5);
	gtk_widget_set_margin_start (STYLIZED_BAR_GRAPH_TO_WIDGET (&page->voltages), 5);
	gtk_widget_set_margin_end (STYLIZED_BAR_GRAPH_TO_WIDGET (&page->voltages), 5);
	gtk_grid_attach (GTK_GRID (grid), STYLIZED_BAR_GRAPH_TO_WIDGET (&page->voltages), 0, 3, 2, 1);

	label = gtk_label_new ("Cell Sense Line Temperatures:");
	gtkLabelSetFont (GTK_LABEL (label), page->style.senseLineTemperatureGraphTitleFont);
	gtkLabelSetColor (GTK_LABEL (label), &page->style.senseLineTemperatureGraphTitleColor);
	gtk_widget_set_halign (label, GTK_ALIGN_START);
	gtk_grid_attach (GTK_GRID (grid), label, 0, 4, 1, 1);

	stylizedBarGraphInit (&page->temperatures, &(stylizedBarGraphConfig_t)
	{
		.accessor			= bmsSenseLineTemperatureGraphAccessor,
		.foregroundDraw		= bmsSenseLineTemperatureGraphDrawForeground,
		.offset				= 0,
		.count				= bmsGetLogicalTemperatureCount (&page->bms),
		.barSize			= 3.5f,
		.barSpacing			= 0.5f,
		.length				= 60,
		.min				= page->bms.minTemperature,
		.max				= page->bms.maxTemperature,
		.tickSpacing		= 10,
		.tickFormat			= "%.0f C",
		.axisPosition		= 40,
		.axisColor			= page->style.senseLineTemperatureGraphAxisColor,
		.tickColor			= page->style.senseLineTemperatureGraphTickColor,
		.tickColor2			= page->style.senseLineTemperatureGraphTickColor2,
		.validBarColor		= page->style.senseLineTemperatureGraphValidBarColor,
		.invalidBarColor	= page->style.senseLineTemperatureGraphInvalidBarColor
	}, &page->bms);
	gtk_widget_set_halign (STYLIZED_BAR_GRAPH_TO_WIDGET (&page->temperatures), GTK_ALIGN_FILL);
	gtk_widget_set_valign (STYLIZED_BAR_GRAPH_TO_WIDGET (&page->temperatures), GTK_ALIGN_FILL);
	gtk_widget_set_hexpand (STYLIZED_BAR_GRAPH_TO_WIDGET (&page->temperatures), true);
	gtk_widget_set_halign (STYLIZED_BAR_GRAPH_TO_WIDGET (&page->temperatures), GTK_ALIGN_FILL);
	gtk_widget_set_vexpand (STYLIZED_BAR_GRAPH_TO_WIDGET (&page->temperatures), true);
	gtk_widget_set_margin_top (STYLIZED_BAR_GRAPH_TO_WIDGET (&page->temperatures), 5);
	gtk_widget_set_margin_bottom (STYLIZED_BAR_GRAPH_TO_WIDGET (&page->temperatures), 5);
	gtk_widget_set_margin_start (STYLIZED_BAR_GRAPH_TO_WIDGET (&page->temperatures), 5);
	gtk_widget_set_margin_end (STYLIZED_BAR_GRAPH_TO_WIDGET (&page->temperatures), 5);
	gtk_grid_attach (GTK_GRID (grid), STYLIZED_BAR_GRAPH_TO_WIDGET (&page->temperatures), 0, 5, 1, 1);

	label = gtk_label_new ("Sense Board IC Temperatures:");
	gtkLabelSetFont (GTK_LABEL (label), page->style.ltcTemperatureGraphTitleFont);
	gtkLabelSetColor (GTK_LABEL (label), &page->style.ltcTemperatureGraphTitleColor);
	gtk_widget_set_halign (label, GTK_ALIGN_START);
	gtk_grid_attach (GTK_GRID (grid), label, 1, 4, 1, 1);

	stylizedBarGraphInit (&page->ltcTemperatures, &(stylizedBarGraphConfig_t)
	{
		.accessor			= bmsLtcTemperatureGraphAccessor,
		.foregroundDraw		= bmsLtcTemperatureGraphDrawForeground,
		.offset				= 0,
		.count				= page->bms.ltcCount,
		.barSize			= 3.5f,
		.barSpacing			= 0.5f,
		.length				= 60,
		.min				= page->bms.minTemperature,
		.max				= page->bms.maxLtcTemperature,
		.tickSpacing		= 10,
		.tickFormat			= "%.0f C",
		.axisPosition		= 40,
		.axisColor			= page->style.ltcTemperatureGraphAxisColor,
		.tickColor			= page->style.ltcTemperatureGraphTickColor,
		.tickColor2			= page->style.ltcTemperatureGraphTickColor2,
		.validBarColor		= page->style.ltcTemperatureGraphValidBarColor,
		.invalidBarColor	= page->style.ltcTemperatureGraphInvalidBarColor
	}, &page->bms);
	gtk_widget_set_halign (STYLIZED_BAR_GRAPH_TO_WIDGET (&page->ltcTemperatures), GTK_ALIGN_FILL);
	gtk_widget_set_valign (STYLIZED_BAR_GRAPH_TO_WIDGET (&page->ltcTemperatures), GTK_ALIGN_FILL);
	gtk_widget_set_hexpand (STYLIZED_BAR_GRAPH_TO_WIDGET (&page->ltcTemperatures), true);
	gtk_widget_set_halign (STYLIZED_BAR_GRAPH_TO_WIDGET (&page->ltcTemperatures), GTK_ALIGN_FILL);
	gtk_widget_set_vexpand (STYLIZED_BAR_GRAPH_TO_WIDGET (&page->ltcTemperatures), true);
	gtk_widget_set_margin_top (STYLIZED_BAR_GRAPH_TO_WIDGET (&page->ltcTemperatures), 5);
	gtk_widget_set_margin_bottom (STYLIZED_BAR_GRAPH_TO_WIDGET (&page->ltcTemperatures), 5);
	gtk_widget_set_margin_start (STYLIZED_BAR_GRAPH_TO_WIDGET (&page->ltcTemperatures), 5);
	gtk_widget_set_margin_end (STYLIZED_BAR_GRAPH_TO_WIDGET (&page->ltcTemperatures), 5);
	gtk_grid_attach (GTK_GRID (grid), STYLIZED_BAR_GRAPH_TO_WIDGET (&page->ltcTemperatures), 1, 5, 1, 1);

	page->buttonPanel = GTK_GRID (gtk_grid_new ());
	gtk_widget_set_margin_bottom (GTK_WIDGET (page->buttonPanel), 10);
	gtk_widget_set_margin_start (GTK_WIDGET (page->buttonPanel), 100);
	gtk_widget_set_margin_end (GTK_WIDGET (page->buttonPanel), 20);
	gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (page->buttonPanel), 0, 6, 2, 1);

	return (page_t*) page;
}
// Header
#include "page_bms_overview.h"

// Includes
#include "gtk_util.h"

page_t* pageBmsOverviewInit (canDatabase_t* database, bms_t* bms)
{
	pageBmsOverview_t* page = malloc (sizeof (pageBmsOverview_t));
	if (page == NULL)
		return NULL;

	// Setup the VMT
	page->vmt.update = pageBmsOverviewUpdate;
	page->vmt.widget = gtk_grid_new ();

	page->bms = bms;

	GtkWidget* label = gtk_label_new ("Statistics:");
	gtk_widget_set_hexpand (label, true);
	gtk_widget_set_halign (label, GTK_ALIGN_START);
	gtk_grid_attach (GTK_GRID (page->vmt.widget), label, 0, 0, 2, 1);

	GtkWidget* statisticPanel = gtk_grid_new ();
	gtk_grid_attach (GTK_GRID (page->vmt.widget), statisticPanel, 0, 1, 2, 1);

	label = gtk_label_new ("Voltage: ");
	page->voltageLabel = (canLabelFloatStatic_t)
	{
		.formatValue	= "%.1f %s",
		.formatInvalid	= "%s %s"
	};
	canLabelFloatStaticInit (&page->voltageLabel);
	gtk_grid_attach (GTK_GRID (statisticPanel), label, 0, 0, 1, 1);
	gtk_grid_attach (GTK_GRID (statisticPanel), CAN_LABEL_FLOAT_STATIC_TO_WIDGET (&page->voltageLabel), 1, 0, 1, 1);
	gtk_widget_set_hexpand (label, true);
	gtk_widget_set_halign (label, GTK_ALIGN_END);
	gtk_widget_set_hexpand (CAN_LABEL_FLOAT_STATIC_TO_WIDGET (&page->voltageLabel), true);
	gtk_widget_set_halign (CAN_LABEL_FLOAT_STATIC_TO_WIDGET (&page->voltageLabel), GTK_ALIGN_START);

	label = gtk_label_new ("Current: ");
	page->currentLabel = (canLabelFloatStatic_t)
	{
		.formatValue	= "%.1f %s",
		.formatInvalid	= "%s %s"
	};
	canLabelFloatStaticInit (&page->currentLabel);
	gtk_grid_attach (GTK_GRID (statisticPanel), label, 2, 0, 1, 1);
	gtk_grid_attach (GTK_GRID (statisticPanel), CAN_LABEL_FLOAT_STATIC_TO_WIDGET (&page->currentLabel), 3, 0, 1, 1);
	gtk_widget_set_hexpand (label, true);
	gtk_widget_set_halign (label, GTK_ALIGN_END);
	gtk_widget_set_hexpand (CAN_LABEL_FLOAT_STATIC_TO_WIDGET (&page->currentLabel), true);
	gtk_widget_set_halign (CAN_LABEL_FLOAT_STATIC_TO_WIDGET (&page->currentLabel), GTK_ALIGN_START);

	label = gtk_label_new ("Power: ");
	page->powerLabel = (canLabelFloatStatic_t)
	{
		.formatValue	= "%.1f %s",
		.formatInvalid	= "%s %s"
	};
	canLabelFloatStaticInit (&page->powerLabel);
	gtk_grid_attach (GTK_GRID (statisticPanel), label, 4, 0, 1, 1);
	gtk_grid_attach (GTK_GRID (statisticPanel), CAN_LABEL_FLOAT_STATIC_TO_WIDGET (&page->powerLabel), 5, 0, 1, 1);
	gtk_widget_set_hexpand (label, true);
	gtk_widget_set_halign (label, GTK_ALIGN_END);
	gtk_widget_set_hexpand (CAN_LABEL_FLOAT_STATIC_TO_WIDGET (&page->powerLabel), true);
	gtk_widget_set_halign (CAN_LABEL_FLOAT_STATIC_TO_WIDGET (&page->powerLabel), GTK_ALIGN_START);

	label = gtk_label_new ("Min Cell: ");
	page->minCellLabel = (canLabelFloatStatic_t)
	{
		.formatValue	= "%.3f %s",
		.formatInvalid	= "%s %s"
	};
	canLabelFloatStaticInit (&page->minCellLabel);
	gtk_grid_attach (GTK_GRID (statisticPanel), label, 0, 1, 1, 1);
	gtk_grid_attach (GTK_GRID (statisticPanel), CAN_LABEL_FLOAT_STATIC_TO_WIDGET (&page->minCellLabel), 1, 1, 1, 1);
	gtk_widget_set_hexpand (label, true);
	gtk_widget_set_halign (label, GTK_ALIGN_END);
	gtk_widget_set_hexpand (CAN_LABEL_FLOAT_STATIC_TO_WIDGET (&page->minCellLabel), true);
	gtk_widget_set_halign (CAN_LABEL_FLOAT_STATIC_TO_WIDGET (&page->minCellLabel), GTK_ALIGN_START);

	label = gtk_label_new ("Max Cell: ");
	page->maxCellLabel = (canLabelFloatStatic_t)
	{
		.formatValue	= "%.3f %s",
		.formatInvalid	= "%s %s"
	};
	canLabelFloatStaticInit (&page->maxCellLabel);
	gtk_grid_attach (GTK_GRID (statisticPanel), label, 2, 1, 1, 1);
	gtk_grid_attach (GTK_GRID (statisticPanel), CAN_LABEL_FLOAT_STATIC_TO_WIDGET (&page->maxCellLabel), 3, 1, 1, 1);
	gtk_widget_set_hexpand (label, true);
	gtk_widget_set_halign (label, GTK_ALIGN_END);
	gtk_widget_set_hexpand (CAN_LABEL_FLOAT_STATIC_TO_WIDGET (&page->maxCellLabel), true);
	gtk_widget_set_halign (CAN_LABEL_FLOAT_STATIC_TO_WIDGET (&page->maxCellLabel), GTK_ALIGN_START);

	label = gtk_label_new ("Avg Cell: ");
	page->avgCellLabel = (canLabelFloatStatic_t)
	{
		.formatValue	= "%.3f %s",
		.formatInvalid	= "%s %s"
	};
	canLabelFloatStaticInit (&page->avgCellLabel);
	gtk_grid_attach (GTK_GRID (statisticPanel), label, 4, 1, 1, 1);
	gtk_grid_attach (GTK_GRID (statisticPanel), CAN_LABEL_FLOAT_STATIC_TO_WIDGET (&page->avgCellLabel), 5, 1, 1, 1);
	gtk_widget_set_hexpand (label, true);
	gtk_widget_set_halign (label, GTK_ALIGN_END);
	gtk_widget_set_hexpand (CAN_LABEL_FLOAT_STATIC_TO_WIDGET (&page->avgCellLabel), true);
	gtk_widget_set_halign (CAN_LABEL_FLOAT_STATIC_TO_WIDGET (&page->avgCellLabel), GTK_ALIGN_START);

	label = gtk_label_new ("Min Temp: ");
	page->minTempLabel = (canLabelFloatStatic_t)
	{
		.formatValue	= "%.1f %s",
		.formatInvalid	= "%s %s"
	};
	canLabelFloatStaticInit (&page->minTempLabel);
	gtk_grid_attach (GTK_GRID (statisticPanel), label, 0, 2, 1, 1);
	gtk_grid_attach (GTK_GRID (statisticPanel), CAN_LABEL_FLOAT_STATIC_TO_WIDGET (&page->minTempLabel), 1, 2, 1, 1);
	gtk_widget_set_hexpand (label, true);
	gtk_widget_set_halign (label, GTK_ALIGN_END);
	gtk_widget_set_hexpand (CAN_LABEL_FLOAT_STATIC_TO_WIDGET (&page->minTempLabel), true);
	gtk_widget_set_halign (CAN_LABEL_FLOAT_STATIC_TO_WIDGET (&page->minTempLabel), GTK_ALIGN_START);

	label = gtk_label_new ("Max Temp: ");
	page->maxTempLabel = (canLabelFloatStatic_t)
	{
		.formatValue	= "%.1f %s",
		.formatInvalid	= "%s %s"
	};
	canLabelFloatStaticInit (&page->maxTempLabel);
	gtk_grid_attach (GTK_GRID (statisticPanel), label, 2, 2, 1, 1);
	gtk_grid_attach (GTK_GRID (statisticPanel), CAN_LABEL_FLOAT_STATIC_TO_WIDGET (&page->maxTempLabel), 3, 2, 1, 1);
	gtk_widget_set_hexpand (label, true);
	gtk_widget_set_halign (label, GTK_ALIGN_END);
	gtk_widget_set_hexpand (CAN_LABEL_FLOAT_STATIC_TO_WIDGET (&page->maxTempLabel), true);
	gtk_widget_set_halign (CAN_LABEL_FLOAT_STATIC_TO_WIDGET (&page->maxTempLabel), GTK_ALIGN_START);

	label = gtk_label_new ("Avg Temp: ");
	page->avgTempLabel = (canLabelFloatStatic_t)
	{
		.formatValue	= "%.1f %s",
		.formatInvalid	= "%s %s"
	};
	canLabelFloatStaticInit (&page->avgTempLabel);
	gtk_grid_attach (GTK_GRID (statisticPanel), label, 4, 2, 1, 1);
	gtk_grid_attach (GTK_GRID (statisticPanel), CAN_LABEL_FLOAT_STATIC_TO_WIDGET (&page->avgTempLabel), 5, 2, 1, 1);
	gtk_widget_set_hexpand (label, true);
	gtk_widget_set_halign (label, GTK_ALIGN_END);
	gtk_widget_set_hexpand (CAN_LABEL_FLOAT_STATIC_TO_WIDGET (&page->avgTempLabel), true);
	gtk_widget_set_halign (CAN_LABEL_FLOAT_STATIC_TO_WIDGET (&page->avgTempLabel), GTK_ALIGN_START);

	label = gtk_label_new ("Max Delta: ");
	page->maxDeltaLabel = (canLabelFloatStatic_t)
	{
		.formatValue	= "%.3f %s",
		.formatInvalid	= "%s %s"
	};
	canLabelFloatStaticInit (&page->maxDeltaLabel);
	gtk_grid_attach (GTK_GRID (statisticPanel), label, 0, 3, 1, 1);
	gtk_grid_attach (GTK_GRID (statisticPanel), CAN_LABEL_FLOAT_STATIC_TO_WIDGET (&page->maxDeltaLabel), 1, 3, 1, 1);
	gtk_widget_set_hexpand (label, true);
	gtk_widget_set_halign (label, GTK_ALIGN_END);
	gtk_widget_set_hexpand (CAN_LABEL_FLOAT_STATIC_TO_WIDGET (&page->maxDeltaLabel), true);
	gtk_widget_set_halign (CAN_LABEL_FLOAT_STATIC_TO_WIDGET (&page->maxDeltaLabel), GTK_ALIGN_START);

	label = gtk_label_new ("Avg Delta: ");
	page->avgDeltaLabel = (canLabelFloatStatic_t)
	{
		.formatValue	= "%.3f %s",
		.formatInvalid	= "%s %s"
	};
	canLabelFloatStaticInit (&page->avgDeltaLabel);
	gtk_grid_attach (GTK_GRID (statisticPanel), label, 2, 3, 1, 1);
	gtk_grid_attach (GTK_GRID (statisticPanel), CAN_LABEL_FLOAT_STATIC_TO_WIDGET (&page->avgDeltaLabel), 3, 3, 1, 1);
	gtk_widget_set_hexpand (label, true);
	gtk_widget_set_halign (label, GTK_ALIGN_END);
	gtk_widget_set_hexpand (CAN_LABEL_FLOAT_STATIC_TO_WIDGET (&page->avgDeltaLabel), true);
	gtk_widget_set_halign (CAN_LABEL_FLOAT_STATIC_TO_WIDGET (&page->avgDeltaLabel), GTK_ALIGN_START);

	label = gtk_label_new ("Cell Voltages:");
	gtk_widget_set_hexpand (label, true);
	gtk_widget_set_halign (label, GTK_ALIGN_START);
	gtk_grid_attach (GTK_GRID (page->vmt.widget), label, 0, 2, 2, 1);

	bmsBarGraphInit (&page->voltages, bms, &(bmsBarGraphConfig_t)
	{
		.accessor		= bmsGetCellVoltage,
		.offset			= 0,
		.count			= bms->cellCount,
		.barSize		= 3.5f,
		.barSpacing		= 0.5f,
		.length			= 100,
		.min			= bms->minCellVoltage,
		.max			= bms->maxCellVoltage,
		.tickSpacing	= 0.25,
		.tickFormat		= "%.2f V",
		.axisPosition	= 40,
		.axisColor		= gdkHexToColor ("#AAAAFF"),
		.tickColor		= gdkHexToColor ("#AAAAAA"),
		.barColor		= gdkHexToColor ("#FFFFFF")
	});
	gtk_grid_attach (GTK_GRID (page->vmt.widget), BMS_BAR_GRAPH_TO_WIDGET (&page->voltages), 0, 3, 2, 1);
	gtk_widget_set_halign (BMS_BAR_GRAPH_TO_WIDGET (&page->voltages), GTK_ALIGN_FILL);
	gtk_widget_set_valign (BMS_BAR_GRAPH_TO_WIDGET (&page->voltages), GTK_ALIGN_FILL);
	gtk_widget_set_hexpand (BMS_BAR_GRAPH_TO_WIDGET (&page->voltages), true);
	gtk_widget_set_vexpand (BMS_BAR_GRAPH_TO_WIDGET (&page->voltages), true);
	gtk_widget_set_margin_top (BMS_BAR_GRAPH_TO_WIDGET (&page->voltages), 5);
	gtk_widget_set_margin_bottom (BMS_BAR_GRAPH_TO_WIDGET (&page->voltages), 5);
	gtk_widget_set_margin_start (BMS_BAR_GRAPH_TO_WIDGET (&page->voltages), 5);
	gtk_widget_set_margin_end (BMS_BAR_GRAPH_TO_WIDGET (&page->voltages), 5);

	label = gtk_label_new ("Sense Line Temperatures:");
	gtk_widget_set_hexpand (label, true);
	gtk_widget_set_halign (label, GTK_ALIGN_START);
	gtk_grid_attach (GTK_GRID (page->vmt.widget), label, 0, 4, 1, 1);

	bmsBarGraphInit (&page->temperatures, bms, &(bmsBarGraphConfig_t)
	{
		.accessor		= bmsGetLogicalTemperature,
		.offset			= 0,
		.count			= bmsGetLogicalTemperatureCount (bms),
		.barSize		= 3.5f,
		.barSpacing		= 0.5f,
		.length			= 100,
		.min			= bms->minTemperature,
		.max			= bms->maxTemperature,
		.tickSpacing	= 10,
		.tickFormat		= "%.0f C",
		.axisPosition	= 40,
		.axisColor		= gdkHexToColor ("#AAAAFF"),
		.tickColor		= gdkHexToColor ("#AAAAAA"),
		.barColor		= gdkHexToColor ("#FFFFFF")
	});
	gtk_grid_attach (GTK_GRID (page->vmt.widget), BMS_BAR_GRAPH_TO_WIDGET (&page->temperatures), 0, 5, 1, 1);
	gtk_widget_set_halign (BMS_BAR_GRAPH_TO_WIDGET (&page->temperatures), GTK_ALIGN_FILL);
	gtk_widget_set_valign (BMS_BAR_GRAPH_TO_WIDGET (&page->temperatures), GTK_ALIGN_FILL);
	gtk_widget_set_hexpand (BMS_BAR_GRAPH_TO_WIDGET (&page->temperatures), true);
	gtk_widget_set_vexpand (BMS_BAR_GRAPH_TO_WIDGET (&page->temperatures), true);
	gtk_widget_set_margin_top (BMS_BAR_GRAPH_TO_WIDGET (&page->temperatures), 5);
	gtk_widget_set_margin_bottom (BMS_BAR_GRAPH_TO_WIDGET (&page->temperatures), 5);
	gtk_widget_set_margin_start (BMS_BAR_GRAPH_TO_WIDGET (&page->temperatures), 5);
	gtk_widget_set_margin_end (BMS_BAR_GRAPH_TO_WIDGET (&page->temperatures), 5);

	label = gtk_label_new ("LTC Temperatures:");
	gtk_widget_set_hexpand (label, true);
	gtk_widget_set_halign (label, GTK_ALIGN_START);
	gtk_grid_attach (GTK_GRID (page->vmt.widget), label, 1, 4, 1, 1);

	bmsBarGraphInit (&page->ltcTemperatures, bms, &(bmsBarGraphConfig_t)
	{
		.accessor		= bmsGetLtcTemperature,
		.offset			= 0,
		.count			= bms->ltcCount,
		.barSize		= 3.5f,
		.barSpacing		= 0.5f,
		.length			= 100,
		.min			= bms->minTemperature,
		.max			= bms->maxLtcTemperature,
		.tickSpacing	= 10,
		.tickFormat		= "%.0f C",
		.axisPosition	= 40,
		.axisColor		= gdkHexToColor ("#AAAAFF"),
		.tickColor		= gdkHexToColor ("#AAAAAA"),
		.barColor		= gdkHexToColor ("#FFFFFF")
	});
	gtk_grid_attach (GTK_GRID (page->vmt.widget), BMS_BAR_GRAPH_TO_WIDGET (&page->ltcTemperatures), 1, 5, 1, 1);
	gtk_widget_set_halign (BMS_BAR_GRAPH_TO_WIDGET (&page->ltcTemperatures), GTK_ALIGN_FILL);
	gtk_widget_set_valign (BMS_BAR_GRAPH_TO_WIDGET (&page->ltcTemperatures), GTK_ALIGN_FILL);
	gtk_widget_set_hexpand (BMS_BAR_GRAPH_TO_WIDGET (&page->ltcTemperatures), true);
	gtk_widget_set_vexpand (BMS_BAR_GRAPH_TO_WIDGET (&page->ltcTemperatures), true);
	gtk_widget_set_margin_top (BMS_BAR_GRAPH_TO_WIDGET (&page->ltcTemperatures), 5);
	gtk_widget_set_margin_bottom (BMS_BAR_GRAPH_TO_WIDGET (&page->ltcTemperatures), 5);
	gtk_widget_set_margin_start (BMS_BAR_GRAPH_TO_WIDGET (&page->ltcTemperatures), 5);
	gtk_widget_set_margin_end (BMS_BAR_GRAPH_TO_WIDGET (&page->ltcTemperatures), 5);

	label = gtk_label_new ("Status:");
	gtk_widget_set_hexpand (label, true);
	gtk_widget_set_halign (label, GTK_ALIGN_START);
	gtk_grid_attach (GTK_GRID (page->vmt.widget), label, 0, 6, 2, 1);

	GtkWidget* statusPanel = gtk_grid_new ();
	gtk_grid_attach (GTK_GRID (page->vmt.widget), statusPanel, 0, 7, 2, 1);

	page->statusCount = bmsGetStatusCount (bms);
	page->statusLabels = malloc (sizeof (canLabelFloat_t) * page->statusCount);
	for (size_t index = 0; index < page->statusCount; ++index)
	{
		char* name = bmsGetStatusName (bms, index);

		int column	= 2 * (index % 3);
		int row		= index / 3;
		GtkWidget* label = gtk_label_new (name);
		gtk_grid_attach (GTK_GRID (statusPanel), label, column, row, 1, 1);
		gtk_widget_set_hexpand (label, true);
		gtk_widget_set_halign (label, GTK_ALIGN_END);

		page->statusLabels [index] = (canLabelFloat_t)
		{
			.signalName		= name,
			.formatValue	= ": %.0f",
			.formatInvalid	= ": %s"
		};
		canLabelFloatInit (&page->statusLabels [index], database);
		gtk_widget_set_halign (CAN_LABEL_FLOAT_TO_WIDGET (&page->statusLabels [index]), GTK_ALIGN_START);
		gtk_grid_attach (GTK_GRID (statusPanel), CAN_LABEL_FLOAT_TO_WIDGET (&page->statusLabels [index]), column + 1, row, 1, 1);
	}

	return (page_t*) page;
}

void pageBmsOverviewUpdate (void* page)
{
	pageBmsOverview_t* pageBms = page;

	bmsBarGraphUpdate (&pageBms->voltages);
	bmsBarGraphUpdate (&pageBms->temperatures);
	bmsBarGraphUpdate (&pageBms->ltcTemperatures);
	for (size_t index = 0; index < pageBms->statusCount; ++index)
		canLabelFloatUpdate (&pageBms->statusLabels [index]);

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
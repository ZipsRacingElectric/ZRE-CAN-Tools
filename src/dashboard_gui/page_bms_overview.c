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

	GtkWidget* statusPanel = gtk_grid_new ();
	gtk_grid_attach (GTK_GRID (page->vmt.widget), statusPanel, 0, 0, 1, 1);

	size_t statusCount = bmsGetStatusCount (bms);
	page->statusLabels = malloc (sizeof (canLabelFloat_t) * statusCount);
	for (size_t index = 0; index < statusCount; ++index)
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
			.formatValue	= ": %f %s",
			.formatInvalid	= ": %s %s"
		};
		canLabelFloatInit (&page->statusLabels [index], database);

		gtk_grid_attach (GTK_GRID (statusPanel), CAN_LABEL_FLOAT_TO_WIDGET (&page->statusLabels [index]), column + 1, row, 1, 1);
	}

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
	gtk_grid_attach (GTK_GRID (page->vmt.widget), BMS_BAR_GRAPH_TO_WIDGET (&page->voltages), 0, 1, 1, 1);
	gtk_widget_set_halign (BMS_BAR_GRAPH_TO_WIDGET (&page->voltages), GTK_ALIGN_FILL);
	gtk_widget_set_valign (BMS_BAR_GRAPH_TO_WIDGET (&page->voltages), GTK_ALIGN_FILL);
	gtk_widget_set_hexpand (BMS_BAR_GRAPH_TO_WIDGET (&page->voltages), true);
	gtk_widget_set_vexpand (BMS_BAR_GRAPH_TO_WIDGET (&page->voltages), true);
	gtk_widget_set_margin_top (BMS_BAR_GRAPH_TO_WIDGET (&page->voltages), 5);
	gtk_widget_set_margin_bottom (BMS_BAR_GRAPH_TO_WIDGET (&page->voltages), 5);
	gtk_widget_set_margin_start (BMS_BAR_GRAPH_TO_WIDGET (&page->voltages), 5);
	gtk_widget_set_margin_end (BMS_BAR_GRAPH_TO_WIDGET (&page->voltages), 5);

	bmsBarGraphInit (&page->temperatures, bms, &(bmsBarGraphConfig_t)
	{
		.accessor		= bmsGetSenseLineTemperature,
		.offset			= 0,
		.count			= bms->senseLineCount,
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
	gtk_grid_attach (GTK_GRID (page->vmt.widget), BMS_BAR_GRAPH_TO_WIDGET (&page->temperatures), 0, 2, 1, 1);
	gtk_widget_set_halign (BMS_BAR_GRAPH_TO_WIDGET (&page->temperatures), GTK_ALIGN_FILL);
	gtk_widget_set_valign (BMS_BAR_GRAPH_TO_WIDGET (&page->temperatures), GTK_ALIGN_FILL);
	gtk_widget_set_hexpand (BMS_BAR_GRAPH_TO_WIDGET (&page->temperatures), true);
	gtk_widget_set_vexpand (BMS_BAR_GRAPH_TO_WIDGET (&page->temperatures), true);
	gtk_widget_set_margin_top (BMS_BAR_GRAPH_TO_WIDGET (&page->temperatures), 5);
	gtk_widget_set_margin_bottom (BMS_BAR_GRAPH_TO_WIDGET (&page->temperatures), 5);
	gtk_widget_set_margin_start (BMS_BAR_GRAPH_TO_WIDGET (&page->temperatures), 5);
	gtk_widget_set_margin_end (BMS_BAR_GRAPH_TO_WIDGET (&page->temperatures), 5);

	return (page_t*) page;
}

void pageBmsOverviewUpdate (void* page)
{
	pageBmsOverview_t* pageBms = page;

	bmsBarGraphUpdate (&pageBms->voltages);
	bmsBarGraphUpdate (&pageBms->temperatures);
}
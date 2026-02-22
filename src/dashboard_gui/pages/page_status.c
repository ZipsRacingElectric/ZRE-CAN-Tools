// Header
#include "page_status.h"

// Includes
#include "../stylized_widgets/stylized_button.h"
#include "../can_widgets/can_signal_indicator.h"
#include "../gtk_util.h"
#include "cjson/cjson_util.h"

static void styleLoad (pageStatusStyle_t* style, pageStyle_t* baseStyle, cJSON* config)
{
	*style = (pageStatusStyle_t)
	{
		.baseStyle = baseStyle
	};

	if (config == NULL)
		return;

	style->baseStyle = pageStyleLoad (jsonGetObjectV2 (config, "baseStyle"), baseStyle);
}

// TODO: Come up with better name for function
// Abstract: this function finds the first failed signal under the "parent signal" and aligns it next to its "parent" along with the error code.
static void updateSignal (GtkGrid* grid, canWidget_t** signals, size_t signalCount)
{
	for (size_t signalIndex = 0; signalIndex < signalCount; ++signalIndex)
	{
		// TODO: make verbose statement
		printf ("%zu). Signal State: %d\n", signalIndex, canSignalIndicatorGetState (signals[signalIndex]));

		// TODO: remove the "|| true" statement.
		if (canSignalIndicatorGetState (signals[signalIndex]) == false || true)
		{
			canWidget_t* signal = signals[signalIndex];
			gtk_grid_attach (grid, CAN_WIDGET_TO_WIDGET (signal), 1, 0, 1, 1);
			return;
		}
	}
}

static void appendButton (void* pageArg, const char* label, pageButtonCallback_t* callback, void* arg, bool currentPage, pageStyle_t* style)
{
	pageStatus_t* page = pageArg;

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

// Note: update function for the selected page is called via vmt->update(page)
// 		=> every parameter to the function needs to be contained in the StatusPage_t* object.
static void update (void* pageArg)
{
	pageStatus_t* page = pageArg;

	// Update the VCU Status
	updateSignal (page->vcuStatusPanel, page->vcuStatusSignals, page->vcuStatusSignalsCount);

}

page_t* pageStatusLoad (cJSON* config, canDatabase_t* database, pageStyle_t* style)
{
	// Fault Devices:
		// Everything Else: Fault Status
			// VCU (pick 4 signals to work with, APPS_1, APPS_2, BSE_f / BSE_fault, BSE_r / BSE_rear)
				// VCU Status Message
					// uses plausible instead of fault (inverse of fault)
			// BMS
			// (any signal in the can database that ends in fault and is a boolean)

		// Fault Code:
			// AMK
				// RL
				// RR
				// FL
				// FR

		// Everything Else: Fault Status
			// GPS
				// Good and bad states (read through documentation)
			// BSPD
				// Signal is not in there?
				// 0/1
				// Implement as boolean Can Label

		// ?
		// Sensor Input Boards
			// Nothing for it yet

	if (config == NULL)
		return NULL;

	char* pageName;
	if (jsonGetString (config, "name", &pageName) != 0)
		return NULL;

	// Allocate the page
	pageStatus_t* page = malloc (sizeof (pageStatus_t));
	if (page == NULL)
		return NULL;

	// Setup the VMT
	page->vmt = (pageVmt_t)
	{
		.update			= update,
		.appendButton	= appendButton,
		.widget			= gtk_overlay_new (),
		.name			= pageName,
		.parent			= NULL // Note: error?
	};

	// TODO: Remove Signal Enumeration
	printf ("Can Database Signals Enum\n");
	for (size_t i = 0; i < database->signalCount; ++i)
	{
		printf ("%s\n", database->signals[i].name);
	}
	printf ("\n");

	cJSON* styleConfig = jsonGetObjectV2 (config, "style");
	styleLoad (&page->style, style, styleConfig);

	page->buttonCount = 0;

	page->grid = GTK_GRID (gtk_grid_new ());
	gtk_overlay_add_overlay (GTK_OVERLAY (page->vmt.widget), GTK_WIDGET (page->grid));
	gtk_overlay_set_measure_overlay (GTK_OVERLAY (page->vmt.widget), GTK_WIDGET (page->grid), true);

	// VCU Status Panel
	page->vcuStatusPanel = GTK_GRID (gtk_grid_new ());
	gtk_grid_set_row_spacing(GTK_GRID (page->vcuStatusPanel), 10);
	gtk_grid_set_column_spacing(GTK_GRID (page->vcuStatusPanel), 10);
	gtk_grid_attach (page->grid, GTK_WIDGET (page->vcuStatusPanel), 0, 0, 2, 2);

	// VCU Status Label
	GtkWidget* label = gtk_label_new ("VCU Status:");
	gtk_grid_attach(page->vcuStatusPanel, label, 0, 0, 1, 1);

	// VCU Status Signals
	page->vcuStatusSignalsCount = 4;
	char* vcuStatusSignals = {
		"APPS_1_PLAUSIBLE",
		"APPS_2_PLAUSIBLE",
		"BSE_F_PLAUSIBLE",
		"BSE_R_PLAUSIBLE"
	};

	page->vcuStatusSignals = malloc(sizeof(canWidget_t*) * 4);

	for (size_t signalIndex = 0; signalIndex < page->vcuStatusSignalsCount; ++signalIndex)
	{
		page->vcuStatusSignals[signalIndex] = canSignalIndicatorInit (database, &(canSignalIndicatorConfig_t)
		{
			.signalName 	= &vcuStatusSignals [signalIndex],
			.inverted 		= true,
			.width 			= 100,
			.height 		= 42,
			.faultColor 	= page->style.baseStyle->indicatorInactiveColor,
			.noFaultColor 	= page->style.baseStyle->indicatorActiveColor,
			.invalidColor 	= page->style.baseStyle->indicatorInactiveColor
		});
	}

	// Update the VCU Status
	updateSignal (page->vcuStatusPanel, page->vcuStatusSignals, page->vcuStatusSignalsCount);

	// BMS Status


	// Sets Button Panel
	page->buttonPanel = GTK_GRID (gtk_grid_new ());
	gtk_widget_set_margin_bottom (GTK_WIDGET (page->buttonPanel), 10);
	gtk_widget_set_margin_start (GTK_WIDGET (page->buttonPanel), 100);
	gtk_widget_set_margin_end (GTK_WIDGET (page->buttonPanel), 20);
	gtk_grid_attach (GTK_GRID(page->grid), GTK_WIDGET (page->buttonPanel), 0, 12, 1, 1);

	// Return the created page (cast to the base type).
	return (page_t*) page;
}
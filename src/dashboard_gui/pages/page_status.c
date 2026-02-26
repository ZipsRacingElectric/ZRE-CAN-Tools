// Header
#include "page_status.h"

// Includes
#include "../can_widgets/can_label_timer.h"
#include "../stylized_widgets/stylized_button.h"
#include "../gtk_util.h"
#include "cjson/cjson.h"
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

static void drawBg (GtkDrawingArea* area, cairo_t* cr, int width, int height, gpointer arg)
{
	pageStatus_t* page = arg;
	(void) area;
	(void) arg;

	// gdk_cairo_set_source_rgba (cr, &page->style.baseStyle->backgroundColor); // DiBacco: uncomment for background color from config

	const char* BG = "#ffffff";
	GdkRGBA bg = gdkHexToColor (BG);
	gdk_cairo_set_source_rgba (cr, &bg);
	cairo_rectangle (cr, 0, 0, width, height);
	cairo_fill (cr);

	graphene_rect_t bounds;
	if (!gtk_widget_compute_bounds (GTK_WIDGET (page->grid), GTK_WIDGET (area), &bounds))
		bounds = *graphene_rect_zero ();

	float xGMin = 0;
	float xGMax = bounds.origin.x + bounds.size.width;

	float yGMin = bounds.origin.y;
	float yGMax = bounds.origin.y + bounds.size.height;

	cairo_pattern_t* gradient;
	gradient = cairo_pattern_create_linear (xGMin, yGMin, xGMax, yGMin);

	cairo_rectangle (cr, xGMin, yGMin, xGMax - xGMin, yGMax - yGMin);
	cairo_fill (cr);
	cairo_pattern_destroy(gradient);
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

static void update (void* pageArg)
{
	pageStatus_t* page = pageArg;

	canWidgetUpdate (page->timer);
}


page_t* pageStatusLoad (cJSON* config, canDatabase_t* database, pageStyle_t* style)
{
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
		.parent			= NULL
	};

	cJSON* styleConfig = jsonGetObjectV2 (config, "style");
	styleLoad (&page->style, style, styleConfig);

	GtkWidget* bg = gtk_drawing_area_new ();
	gtk_drawing_area_set_draw_func (GTK_DRAWING_AREA (bg), drawBg, page, NULL);
	gtk_overlay_set_child (GTK_OVERLAY (page->vmt.widget), bg);

	page->grid = GTK_GRID (gtk_grid_new ());
	gtk_overlay_add_overlay (GTK_OVERLAY (page->vmt.widget), GTK_WIDGET (page->grid));
	gtk_overlay_set_measure_overlay (GTK_OVERLAY (page->vmt.widget), GTK_WIDGET (page->grid), true);

	page->buttonCount = 0;

	// TODO: Setup GUI.
	(void) database;

	page->timer = canLabelTimerInit (database, &(canLabelTimerConfig_t) {
		.startTime 			= { .tv_sec = 0, .tv_nsec = 0 },
		.currentTime 		= { .tv_sec = 0, .tv_nsec = 0 },
		.width 				= 25,
		.height 			= 50,
		.borderThickness 	= page->style.baseStyle->borderThickness,
		.backgroundColor 	= page->style.baseStyle->backgroundColor,
		.borderColor		= page->style.baseStyle->borderColor,
		.fontColor			= page->style.baseStyle->fontColor
	});

	gtk_grid_attach (GTK_GRID (page->grid), CAN_WIDGET_TO_WIDGET (page->timer), 0, 0, 1, 1);

	page->buttonPanel = GTK_GRID (gtk_grid_new ());
	gtk_widget_set_margin_bottom (GTK_WIDGET (page->buttonPanel), 10);
	gtk_widget_set_margin_start (GTK_WIDGET (page->buttonPanel), 100);
	gtk_widget_set_margin_end (GTK_WIDGET (page->buttonPanel), 20);
	gtk_grid_attach (GTK_GRID (page->grid), GTK_WIDGET (page->buttonPanel), 0, 1, 1, 1);

	// Return the created page (cast to the base type).
	return (page_t*) page;
}
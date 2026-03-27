// Header
#include "page_status.h"

// Includes
#include "../stylized_widgets/stylized_button.h"
#include "../gtk_util.h"
#include "cjson/cjson_util.h"
#include "misc_port.h"

static void drawBg (GtkDrawingArea* area, cairo_t* cr, int width, int height, gpointer arg)
{
	pageStatus_t* page = arg;
	(void) area;
	(void) arg;

	gdk_cairo_set_source_rgba (cr, &page->style.baseStyle->backgroundColor);
	cairo_rectangle (cr, 0, 0, width, height);
	cairo_fill (cr);
}

static void drawShutdownLines (GtkDrawingArea* area, cairo_t* cr, int width, int height, gpointer arg)
{
	pageStatus_t* page = arg;
	(void) area;
	(void) arg;
	(void) height;

	graphene_rect_t bounds;
	if (page->positiveIr == NULL || !gtk_widget_compute_bounds (CAN_WIDGET_TO_WIDGET (page->positiveIr), GTK_WIDGET (area), &bounds))
		bounds = *graphene_rect_zero ();

	float x0 = 0;
	float y0 = bounds.origin.y + bounds.size.height / 2.0f - page->style.shutdownLineThickness;

	float x1 = width;
	float y1 = y0;

	float x2 = x1;
	float y2 = y1 + page->style.shutdownLineThickness;

	float x3 = x0;
	float y3 = y2;

	if (page->negativeIr == NULL || !gtk_widget_compute_bounds (CAN_WIDGET_TO_WIDGET (page->negativeIr), GTK_WIDGET (area), &bounds))
		bounds = *graphene_rect_zero ();

	float y10 = bounds.origin.y + bounds.size.height / 2.0f - page->style.shutdownLineThickness;

	float x11 = width;
	float y11 = y10;

	float x12 = x11;
	float y12 = y11 + page->style.shutdownLineThickness;

	float y13 = y12;

	float x20 = width / 2.0f - page->style.shutdownLineSlope * (y12 - y0) / 2.0f - page->style.shutdownLineThickness / 2.0f;
	float y20 = y0;

	float x21 = x20 + page->style.shutdownLineThickness;
	float y21 = y20;

	float x22 = width / 2.0f + page->style.shutdownLineSlope * (y12 - y0) / 2.0f + page->style.shutdownLineThickness / 2.0f;
	float y22 = y12;

	float x23 = x22 - page->style.shutdownLineThickness;
	float y23 = y12;

	float x10 = x23;
	float x13 = x23;

	gdk_cairo_set_source_rgba (cr, &page->style.shutdownLineColor);

	cairo_move_to (cr, x0, y0);
	cairo_line_to (cr, x1, y1);
	cairo_line_to (cr, x2, y2);
	cairo_line_to (cr, x3, y3);
	cairo_fill (cr);

	cairo_move_to (cr, x10, y10);
	cairo_line_to (cr, x11, y11);
	cairo_line_to (cr, x12, y12);
	cairo_line_to (cr, x13, y13);
	cairo_fill (cr);

	cairo_move_to (cr, x20, y20);
	cairo_line_to (cr, x21, y21);
	cairo_line_to (cr, x22, y22);
	cairo_line_to (cr, x23, y23);
	cairo_fill (cr);
}

static void styleLoad (pageStatusStyle_t* style, pageStyle_t* baseStyle, cJSON* config)
{
	*style = (pageStatusStyle_t)
	{
		.baseStyle = baseStyle
	};

	if (config == NULL)
		return;

	style->baseStyle = pageStyleLoad (jsonGetObjectV2 (config, "baseStyle"), baseStyle);

	char* color;
	if (jsonGetString (config, "shutdownLineColor", &color) == 0)
		style->shutdownLineColor = gdkHexToColor (color);

	jsonGetFloat (config, "shutdownLineThickness", &style->shutdownLineThickness);
	jsonGetFloat (config, "shutdownLineSlope", &style->shutdownLineSlope);
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
			page->style.baseStyle->buttonActiveColor : page->style.baseStyle->buttonInactiveColor
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
	pageStatus_t* page = pageArg;

	canWidgetUpdate (page->positiveIr);
	canWidgetUpdate (page->negativeIr);
	canWidgetUpdateArray (page->shutdownIndicators, page->shutdownIndicatorCount);
	canWidgetUpdateArray (page->diagramIndicators, page->diagramIndicatorCount);
}

page_t* pageStatusLoad (cJSON* config, canDatabase_t* databases, size_t databaseCount, pageStyle_t* style)
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

	page->buttonCount = 0;

	GtkWidget* bg = gtk_drawing_area_new ();
	gtk_drawing_area_set_draw_func (GTK_DRAWING_AREA (bg), drawBg, page, NULL);
	gtk_overlay_set_child (GTK_OVERLAY (page->vmt.widget), bg);

	page->grid = GTK_GRID (gtk_grid_new ());
	gtk_overlay_add_overlay (GTK_OVERLAY (page->vmt.widget), GTK_WIDGET (page->grid));
	gtk_overlay_set_measure_overlay (GTK_OVERLAY (page->vmt.widget), GTK_WIDGET (page->grid), true);

	// Load the shutdown panel

	GtkWidget* shutdownPanel = gtk_grid_new ();
	gtk_widget_set_margin_end (shutdownPanel, 4);
	gtk_grid_attach (GTK_GRID (page->grid), shutdownPanel, 0, 0, 1, 1);

	page->shutdownIndicators = canWidgetLoadArray (databases, databaseCount, jsonGetObjectV2 (config, "shutdownIndicators"),
		PAGE_WIDGET_STYLE (page), &page->shutdownIndicatorCount);
	if (page->shutdownIndicators == NULL)
		return NULL;

	for (size_t index = 0; index < page->shutdownIndicatorCount; ++index)
	{
		if (page->shutdownIndicators [index] == NULL)
			continue;

		GtkWidget* widget = CAN_WIDGET_TO_WIDGET (page->shutdownIndicators [index]);
		gtk_widget_set_margin_top (widget, 8);
		gtk_grid_attach (GTK_GRID (shutdownPanel), widget, index, 0, 1, 2);
	}

	GtkWidget* shutdownLines = gtk_drawing_area_new ();
	gtk_widget_set_hexpand (shutdownLines, true);
	gtk_drawing_area_set_draw_func (GTK_DRAWING_AREA (shutdownLines), drawShutdownLines, page, NULL);
	gtk_grid_attach (GTK_GRID (shutdownPanel), shutdownLines, page->shutdownIndicatorCount, 0, 1, 2);

	cJSON* positiveIr = jsonGetObjectV2 (config, "positiveIr");
	page->positiveIr = canWidgetLoad (databases, databaseCount, positiveIr, PAGE_WIDGET_STYLE (page));
	if (page->positiveIr != NULL)
		gtk_grid_attach (GTK_GRID (shutdownPanel), CAN_WIDGET_TO_WIDGET (page->positiveIr), page->shutdownIndicatorCount + 1, 0, 1, 1);

	cJSON* negativeIr = jsonGetObjectV2 (config, "negativeIr");
	page->negativeIr = canWidgetLoad (databases, databaseCount, negativeIr, PAGE_WIDGET_STYLE (page));
	if (page->negativeIr != NULL)
		gtk_grid_attach (GTK_GRID (shutdownPanel), CAN_WIDGET_TO_WIDGET (page->negativeIr), page->shutdownIndicatorCount + 1, 1, 1, 1);

	// Load the diagram background

	GtkWidget* diagram = gtk_fixed_new ();
	gtk_widget_set_halign (diagram, GTK_ALIGN_CENTER);
	gtk_widget_set_valign (diagram, GTK_ALIGN_CENTER);
	gtk_widget_set_vexpand (diagram, true);
	gtk_grid_attach (GTK_GRID (page->grid), diagram, 0, 8, 1, 1);

	char* backgroundPath;
	if (jsonGetString (config, "diagramBackground", &backgroundPath) != 0)
		return NULL;

	char* backgroundPathExp = expandEnv (backgroundPath);
	if (backgroundPathExp == NULL)
		return NULL;

	GtkWidget* diagramBackground = gtk_picture_new_for_filename (backgroundPathExp);
	gtk_picture_set_can_shrink (GTK_PICTURE (diagramBackground), false);
	gtk_fixed_put (GTK_FIXED (diagram), diagramBackground, 0, 0);

	free (backgroundPathExp);

	// Load the diagram indicators

	cJSON* indicatorConfigs = jsonGetObjectV2 (config, "diagramIndicators");
	if (indicatorConfigs == NULL)
		return NULL;

	page->diagramIndicatorCount = cJSON_GetArraySize (indicatorConfigs);
	page->diagramIndicators = malloc (page->diagramIndicatorCount * sizeof (canWidget_t*));
	if (page->diagramIndicators == NULL)
		return NULL;

	for (size_t index = 0; index < page->diagramIndicatorCount; ++index)
	{
		cJSON* indicatorConfig = cJSON_GetArrayItem (indicatorConfigs, index);

		uint16_t x = 0;
		jsonGetUint16_t (indicatorConfig, "xPosition", &x);

		uint16_t y = 0;
		jsonGetUint16_t (indicatorConfig, "yPosition", &y);

		cJSON* widgetConfig = jsonGetObjectV2 (indicatorConfig, "widget");
		page->diagramIndicators [index] = canWidgetLoad (databases, databaseCount, widgetConfig, PAGE_WIDGET_STYLE (page));

		if (page->diagramIndicators [index] == NULL)
			continue;

		gtk_fixed_put (GTK_FIXED (diagram), CAN_WIDGET_TO_WIDGET (page->diagramIndicators [index]), x, y);
	}

	// Create padding and the button panel

	GtkWidget* padding = gtk_grid_new ();
	gtk_widget_set_vexpand (padding, true);
	gtk_grid_attach (GTK_GRID (page->grid), padding, 0, 9, 1, 1);

	page->buttonPanel = GTK_GRID (gtk_grid_new ());
	gtk_widget_set_margin_bottom (GTK_WIDGET (page->buttonPanel), 10);
	gtk_widget_set_margin_start (GTK_WIDGET (page->buttonPanel), 100);
	gtk_widget_set_margin_end (GTK_WIDGET (page->buttonPanel), 20);
	gtk_grid_attach (GTK_GRID (page->grid), GTK_WIDGET (page->buttonPanel), 0, 10, 1, 1);

	// Return the created page (cast to the base type).
	return (page_t*) page;
}
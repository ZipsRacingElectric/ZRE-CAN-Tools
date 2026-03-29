// Header
#include "page_drive.h"

// Includes
#include "../stylized_widgets/stylized_button.h"
#include "../gtk_util.h"
#include "cjson/cjson_util.h"

static void drawHorizontalDecals (cairo_t* cr, pageDrive_t* page, float x0, float x1, float y, int count, bool direction, bool drawDark, bool drawLight, bool clip)
{
	float spacing = 1.0f / (count - 1.0f) * (x1 - x0);
	float halfWidth = page->style.decalWidth / 2.0f;

	float sign = direction ? 1.0f : -1.0f;

	if (drawLight)
	{
		gdk_cairo_set_source_rgba (cr, &page->style.decalTickColorLight);
		cairo_set_line_width (cr, page->style.decalThickness);
		for (int index = 0; index < count; ++index)
		{
			float x = index * spacing + x0;

			if (!clip || index != 0)
			{
				cairo_move_to (cr, x - halfWidth, y - sign);
				cairo_line_to (cr, x - halfWidth, y - sign * (1 + page->style.decalHeight));
				cairo_stroke (cr);
			}

			if (!clip || index != count - 1)
			{
				cairo_move_to (cr, x + halfWidth, y - sign);
				cairo_line_to (cr, x + halfWidth, y - sign * (1 + page->style.decalHeight));
				cairo_stroke (cr);
			}
		}
	}

	if (drawDark)
	{
		gdk_cairo_set_source_rgba (cr, &page->style.decalTickColorDark);
		for (int index = 0; index < count; ++index)
		{
			float x = index * spacing + x0;

			cairo_move_to (cr, x, y + sign);
			cairo_line_to (cr, x, y + sign * (1 + page->style.decalHeight));
			cairo_stroke (cr);
		}
	}
}

static void drawBg (GtkDrawingArea* area, cairo_t* cr, int width, int height, gpointer arg)
{
	pageDrive_t* page = arg;
	(void) area;
	(void) arg;

	gdk_cairo_set_source_rgba (cr, &page->style.baseStyle->backgroundColor);
	cairo_rectangle (cr, 0, 0, width, height);
	cairo_fill (cr);

	graphene_rect_t bounds;
	if (!gtk_widget_compute_bounds (GTK_WIDGET (page->grid), GTK_WIDGET (area), &bounds))
		bounds = *graphene_rect_zero ();

	float xGMin = 0;
	float yGMin = bounds.origin.y;
	float yGMax = bounds.origin.y + bounds.size.height;

	if (!gtk_widget_compute_bounds (page->rightPanel, GTK_WIDGET (area), &bounds))
		bounds = *graphene_rect_zero ();

	float xGMax = bounds.origin.x + bounds.size.width;

	cairo_pattern_t* gradient;
	gradient = cairo_pattern_create_linear (xGMin, yGMin, xGMax, yGMin);
	gdkCairoPatternAddColorStop (gradient, page->style.gradientStartPosition,	&page->style.gradientStartColor);
	gdkCairoPatternAddColorStop (gradient, page->style.gradientMiddlePosition,	&page->style.gradientMiddleColor);
	gdkCairoPatternAddColorStop (gradient, page->style.gradientEndPosition,	&page->style.gradientEndColor);
	cairo_set_source (cr, gradient);

	cairo_rectangle (cr, xGMin, yGMin, xGMax - xGMin, yGMax - yGMin);
	cairo_fill (cr);
	cairo_pattern_destroy (gradient);

	if (!gtk_widget_compute_bounds (GTK_WIDGET (page->dataLoggerPanel), GTK_WIDGET (area), &bounds))
		bounds = *graphene_rect_zero ();

	float x0 = bounds.origin.x;
	float y0 = bounds.origin.y;

	float x1 = bounds.origin.x + bounds.size.width;
	float y1 = bounds.origin.y;

	if (!gtk_widget_compute_bounds (page->centerPanelTitle, GTK_WIDGET (area), &bounds))
		bounds = *graphene_rect_zero ();

	float x2 = bounds.origin.x;
	float y2 = bounds.origin.y;

	x1 -= (y2 - y1) / 2;

	if (!gtk_widget_compute_bounds (page->rightPanel, GTK_WIDGET (area), &bounds))
		bounds = *graphene_rect_zero ();

	float x3 = bounds.origin.x + bounds.size.width;
	float y3 = y2;

	if (!gtk_widget_compute_bounds (page->centerPanel, GTK_WIDGET (area), &bounds))
		bounds = *graphene_rect_zero ();

	float x4 = x3;
	float y4 = bounds.origin.y + bounds.size.height;

	if (!gtk_widget_compute_bounds (page->leftPanel, GTK_WIDGET (area), &bounds))
		bounds = *graphene_rect_zero ();

	float x5 = bounds.origin.x;
	float y5 = y4;

	if (page->leftProgressBar == NULL || !gtk_widget_compute_bounds (CAN_WIDGET_TO_WIDGET (page->leftProgressBar), GTK_WIDGET (area), &bounds))
		bounds = *graphene_rect_zero ();

	float x6 = bounds.origin.x + bounds.size.width + 30;
	float y6 = yGMax;

	float x7 = x6 - 20;
	float y7 = y6;

	if (!gtk_widget_compute_bounds (GTK_WIDGET (page->dataLoggerPanel), GTK_WIDGET (area), &bounds))
		bounds = *graphene_rect_zero ();

	float x8 = x5 - 30;
	float y8 = bounds.origin.y + bounds.size.height;

	float x9 = bounds.origin.x;
	float y9 = y8;

	float x10 = x5;
	float y10 = y8;

	float y19 = y8;
	float x19 = bounds.origin.x + bounds.size.width - (y19 - y2) / 2;

	if (!gtk_widget_compute_bounds (GTK_WIDGET (page->faultPanel), GTK_WIDGET (area), &bounds))
		bounds = *graphene_rect_zero ();

	float x11 = bounds.origin.x;
	float y11 = bounds.origin.y;

	float x12 = x11 + bounds.size.width;
	float y12 = y11;

	float x13 = x12;
	float y13 = y12 + bounds.size.height;

	float x14 = x11 - (y13 - y11) / 2;
	float y14 = y13;

	if (!gtk_widget_compute_bounds (GTK_WIDGET (page->buttonPanel), GTK_WIDGET (area), &bounds))
		bounds = *graphene_rect_zero ();

	float x15 = bounds.origin.x;
	float y15 = bounds.origin.y;

	float x16 = x15 + bounds.size.width;
	float y16 = y15;

	float x17 = x16;
	float y17 = y16 + bounds.size.height;

	float x18 = x6 + 10;
	float y18 = y17;

	gdk_cairo_set_source_rgba (cr, &page->style.baseStyle->backgroundColor);
	cairo_move_to (cr, x0, y0);
	cairo_line_to (cr, x1, y1);
	cairo_line_to (cr, x2, y2);
	cairo_line_to (cr, x3, y3);
	cairo_line_to (cr, x4, y4);
	cairo_line_to (cr, x5, y5);
	cairo_line_to (cr, x6, y6);
	cairo_line_to (cr, x7, y7);
	cairo_line_to (cr, x8, y8);
	cairo_line_to (cr, x9, y9);
	cairo_fill (cr);

	cairo_move_to (cr, x11, y11);
	cairo_line_to (cr, x12, y12);
	cairo_line_to (cr, x13, y13);
	cairo_line_to (cr, x14, y14);
	cairo_fill (cr);

	cairo_move_to (cr, x15, y15);
	cairo_line_to (cr, x16, y16);
	cairo_line_to (cr, x17, y17);
	cairo_line_to (cr, x18, y18);
	cairo_fill (cr);

	gdk_cairo_set_source_rgba (cr, &page->style.baseStyle->borderColor);
	cairo_set_line_width (cr, page->style.baseStyle->borderThickness);

	cairo_move_to (cr, x10, y10);
	cairo_line_to (cr,  x9,  y9);
	cairo_line_to (cr,  x0,  y0);
	cairo_line_to (cr,  x1,  y1);
	cairo_line_to (cr,  x2,  y2);
	cairo_line_to (cr,  x3,  y3);
	cairo_line_to (cr,  x4,  y4);
	cairo_line_to (cr,  x5,  y5);
	cairo_line_to (cr, x10, y10);
	cairo_line_to (cr, x19, y19);
	cairo_line_to (cr,  x2,  y2);
	cairo_stroke (cr);

	cairo_move_to (cr, x11, y11);
	cairo_line_to (cr, x12, y12);
	cairo_line_to (cr, x13, y13);
	cairo_line_to (cr, x14, y14);
	cairo_line_to (cr, x11, y11);
	cairo_stroke (cr);

	if (!gtk_widget_compute_bounds (page->leftPanel, GTK_WIDGET (area), &bounds))
		bounds = *graphene_rect_zero ();
	cairo_rectangle (cr, bounds.origin.x, bounds.origin.y, bounds.size.width, bounds.size.height);
	cairo_stroke (cr);

	if (!gtk_widget_compute_bounds (page->rightPanel, GTK_WIDGET (area), &bounds))
		bounds = *graphene_rect_zero ();
	cairo_rectangle (cr, bounds.origin.x, bounds.origin.y, bounds.size.width, bounds.size.height);
	cairo_stroke (cr);

	// Upper decals
	float x0DecalUpper = xGMin + page->style.decalUpperTickMargin;
	float x1DecalUpper = xGMax - page->style.decalUpperTickMargin;
	int decalUpperCount = (x1DecalUpper - x0DecalUpper) / page->style.decalUpperTickSpacing;
	drawHorizontalDecals (cr, page, x0DecalUpper, x1DecalUpper, yGMin, decalUpperCount, true, true, true, false);

	// Button decals
	drawHorizontalDecals (cr, page, x15, x16, y15, page->buttonCount + 1, false, true, true, true);

	// Center decals
	float x0DecalCenter = x2 + page->style.decalCenterTickMargin;
	float x1DecalCenter = x3 - page->style.decalCenterTickMargin;
	int decalCenterCount = (x1DecalCenter - x0DecalCenter) / page->style.decalCenterTickSpacing;
	drawHorizontalDecals (cr, page, x0DecalCenter, x1DecalCenter, y2, decalCenterCount, false, true, false, false);

	// Lower decals
	gdk_cairo_set_source_rgba (cr, &page->style.decalTickColorLight);

	cairo_move_to (cr, x6, y6 + 1);
	cairo_line_to (cr, x6, y6 + 1 + page->style.decalHeight);
	cairo_stroke (cr);

	cairo_move_to (cr, x7, y7 + 1);
	cairo_line_to (cr, x7, y7 + 1 + page->style.decalHeight);
	cairo_stroke (cr);
}

static void styleLoad (pageDriveStyle_t* style, pageStyle_t* baseStyle, cJSON* config)
{
	*style = (pageDriveStyle_t)
	{
		.baseStyle = baseStyle
	};

	if (config == NULL)
		return;

	style->baseStyle = pageStyleLoad (jsonGetObjectV2 (config, "baseStyle"), baseStyle);

	char* color;
	if (jsonGetString (config, "decalTickColorLight", &color) == 0)
		style->decalTickColorLight = gdkHexToColor (color);
	if (jsonGetString (config, "decalTickColorDark", &color) == 0)
		style->decalTickColorDark = gdkHexToColor (color);

	jsonGetFloat (config, "decalUpperTickMargin", &style->decalUpperTickMargin);
	jsonGetFloat (config, "decalUpperTickSpacing", &style->decalUpperTickSpacing);
	jsonGetFloat (config, "decalCenterTickMargin", &style->decalCenterTickMargin);
	jsonGetFloat (config, "decalCenterTickSpacing", &style->decalCenterTickSpacing);
	jsonGetFloat (config, "decalHeight", &style->decalHeight);
	jsonGetFloat (config, "decalThickness", &style->decalThickness);
	jsonGetFloat (config, "decalWidth", &style->decalWidth);

	if (jsonGetString (config, "gradientStartColor", &color) == 0)
		style->gradientStartColor = gdkHexToColor (color);
	jsonGetFloat (config, "gradientStartPosition", &style->gradientStartPosition);
	if (jsonGetString (config, "gradientMiddleColor", &color) == 0)
		style->gradientMiddleColor = gdkHexToColor (color);
	jsonGetFloat (config, "gradientMiddlePosition", &style->gradientMiddlePosition);
	if (jsonGetString (config, "gradientEndColor", &color) == 0)
		style->gradientEndColor = gdkHexToColor (color);
	jsonGetFloat (config, "gradientEndPosition", &style->gradientEndPosition);

	jsonGetString (config, "dataLoggerTitleFont", &style->dataLoggerTitleFont);
	jsonGetString (config, "dataLoggerStatFont", &style->dataLoggerStatFont);
	jsonGetString (config, "centerPanelTitleFont", &style->centerPanelTitleFont);
	jsonGetString (config, "centerPanelLabelFont", &style->centerPanelLabelFont);
	jsonGetString (config, "centerPanelStatFont", &style->centerPanelStatFont);
	jsonGetString (config, "sidePanelTitleFont", &style->sidePanelTitleFont);
	jsonGetString (config, "sidePanelStatFont", &style->sidePanelStatFont);
}

static void centerPanelLoad (pageDrive_t* page, cJSON* config, canDatabase_t* databases, size_t databaseCount)
{
	// Check a config was specified for the panel

	page->centerPanelWidgets = NULL;
	page->centerPanelWidgetCount = 0;
	if (config == NULL)
		return;

	// Load the title

	char* title;
	if (jsonGetString (config, "title", &title) == 0)
		gtk_label_set_text (GTK_LABEL (page->centerPanelTitle), title);

	// Create the top padding

	GtkWidget* padding = gtk_grid_new ();
	gtk_widget_set_vexpand (padding, true);
	gtk_grid_attach (GTK_GRID (page->centerPanel), padding, 0, 0, 2, 1);

	// Load the labels

	cJSON* labelConfigs = jsonGetObjectV2 (config, "labels");
	if (labelConfigs == NULL)
		return;

	size_t labelConfigCount = cJSON_GetArraySize (labelConfigs);
	for (size_t index = 0; index < labelConfigCount; ++index)
	{
		cJSON* labelConfig = cJSON_GetArrayItem (labelConfigs, index);
		char* labelValue = cJSON_GetStringValue (labelConfig);

		GtkWidget* label = gtk_label_new (labelValue);
		gtkLabelSetFont (GTK_LABEL (label), page->style.centerPanelLabelFont);
		gtkLabelSetColor (GTK_LABEL (label), &page->style.baseStyle->fontColor);
		gtk_widget_set_halign (label, GTK_ALIGN_START);
		gtk_widget_set_valign (label, GTK_ALIGN_BASELINE_CENTER);
		gtk_widget_set_margin_start (label, 3);
		gtk_grid_attach (GTK_GRID (page->centerPanel), label, 0, index + 1, 1, 1);
	}

	// Load the CAN widgets

	page->centerPanelWidgets = canWidgetLoadArray (databases, databaseCount, jsonGetObjectV2 (config, "widgets"), PAGE_WIDGET_STYLE (page), &page->centerPanelWidgetCount);
	if (page->centerPanelWidgets == NULL)
		return;

	for (size_t index = 0; index < page->centerPanelWidgetCount; ++index)
	{
		if (page->centerPanelWidgets [index] == NULL)
			continue;

		GtkWidget* widget = CAN_WIDGET_TO_WIDGET (page->centerPanelWidgets [index]);

		gtkTryLabelSetFont (widget, page->style.centerPanelStatFont);
		gtkTryLabelSetColor (widget, &page->style.baseStyle->fontColor);

		if (labelConfigCount == 0)
			gtk_widget_set_halign (widget, GTK_ALIGN_CENTER);
		else
			gtk_widget_set_halign (widget, GTK_ALIGN_END);

		gtk_widget_set_hexpand (widget, true);
		gtk_widget_set_valign (widget, GTK_ALIGN_BASELINE_CENTER);
		gtk_widget_set_margin_end (widget, 3);
		gtk_grid_attach (GTK_GRID (page->centerPanel), widget, 1, index + 1, 1, 1);
	}

	// Create the bottom padding

	padding = gtk_grid_new ();
	gtk_widget_set_vexpand (padding, true);
	gtk_grid_attach (GTK_GRID (page->centerPanel), padding, 0, page->centerPanelWidgetCount + 1, 2, 1);
}

static void sidePanelLoad (pageDrive_t* page, cJSON* config, canDatabase_t* databases, size_t databaseCount, GtkWidget** sidePanel, canWidget_t*** widgets, size_t* widgetCount)
{
	// Check a config was specified for the panel

	*widgets = NULL;
	*widgetCount = 0;
	if (config == NULL)
		return;

	// Set the panel's width

	int width = 0;
	jsonGetInt (config, "width", &width);
	gtk_widget_set_size_request (*sidePanel, width, 0);

	// Create the panel's title

	char* title = "";
	jsonGetString (config, "title", &title);
	GtkWidget* label = gtk_label_new (title);
	gtkLabelSetFont (GTK_LABEL (label), page->style.sidePanelTitleFont);
	gtkLabelSetColor (GTK_LABEL (label), &page->style.baseStyle->fontColor);
	gtk_label_set_xalign (GTK_LABEL (label), 0.5);
	gtk_widget_set_size_request (label, width, 0);
	gtk_widget_set_margin_top (label, 3);
	gtk_grid_attach (GTK_GRID (*sidePanel), label, 0, 0, 2, 1);

	// Load the labels

	cJSON* labelConfigs = jsonGetObjectV2 (config, "labels");
	if (labelConfigs == NULL)
		return;

	size_t labelConfigCount = cJSON_GetArraySize (labelConfigs);
	for (size_t index = 0; index < labelConfigCount; ++index)
	{
		cJSON* labelConfig = cJSON_GetArrayItem (labelConfigs, index);
		char* labelValue = cJSON_GetStringValue (labelConfig);

		label = gtk_label_new (labelValue);
		gtkLabelSetFont (GTK_LABEL (label), page->style.sidePanelStatFont);
		gtkLabelSetColor (GTK_LABEL (label), &page->style.baseStyle->fontColor);
		gtk_widget_set_halign (label, GTK_ALIGN_START);
		gtk_widget_set_margin_start (label, 3);
		gtk_grid_attach (GTK_GRID (*sidePanel), label, 0, index + 1, 1, 1);
	}

	// Load the CAN widgets

	*widgets = canWidgetLoadArray (databases, databaseCount, jsonGetObjectV2 (config, "widgets"), PAGE_WIDGET_STYLE (page), widgetCount);
	if (*widgets == NULL)
		return;

	for (size_t index = 0; index < *widgetCount; ++index)
	{
		if ((*widgets) [index] == NULL)
			continue;

		GtkWidget* widget = CAN_WIDGET_TO_WIDGET ((*widgets) [index]);
		gtkTryLabelSetFont (widget, page->style.sidePanelStatFont);
		gtkTryLabelSetColor (widget, &page->style.baseStyle->fontColor);
		gtk_widget_set_halign (widget, GTK_ALIGN_END);
		gtk_widget_set_margin_end (widget, 3);
		gtk_grid_attach (GTK_GRID (*sidePanel), widget, 1, index + 1, 1, 1);
	}
}

static void appendButton (void* pageArg, const char* label, pageButtonCallback_t* callback, void* arg, bool currentPage, pageStyle_t* style)
{
	pageDrive_t* page = pageArg;

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
	pageDrive_t* page = pageArg;

	canWidgetUpdate (page->leftProgressBar);
	canWidgetUpdate (page->rightProgressBar);
	canWidgetUpdate (page->dataLoggerTitle);
	canWidgetUpdate (page->dataLoggerStat);
	canWidgetUpdateArray (page->centerPanelWidgets, page->centerPanelWidgetCount);
	canWidgetUpdateArray (page->leftPanelWidgets, page->leftPanelWidgetCount);
	canWidgetUpdateArray (page->rightPanelWidgets, page->rightPanelWidgetCount);
	canWidgetUpdateArray (page->faultIndicators, page->faultIndicatorCount);
}

page_t* pageDriveLoad (cJSON* config, canDatabase_t* databases, size_t databaseCount, pageStyle_t* style)
{
	if (config == NULL)
		return NULL;

	char* pageName;
	if (jsonGetString (config, "name", &pageName) != 0)
		return NULL;

	// Allocate the page
	pageDrive_t* page = malloc (sizeof (pageDrive_t));
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
	gtk_widget_set_margin_top (GTK_WIDGET (page->grid), 10);
	gtk_widget_set_margin_bottom (GTK_WIDGET (page->grid), 10);
	gtk_overlay_add_overlay (GTK_OVERLAY (page->vmt.widget), GTK_WIDGET (page->grid));
	gtk_overlay_set_measure_overlay (GTK_OVERLAY (page->vmt.widget), GTK_WIDGET (page->grid), true);

	// Load the left progress bar

	cJSON* leftProgressBarConfig = jsonGetObjectV2 (config, "leftProgressBar");
	page->leftProgressBar = canWidgetLoad (databases, databaseCount, leftProgressBarConfig, PAGE_WIDGET_STYLE (page));
	if (page->leftProgressBar != NULL)
	{
		GtkWidget* widget = CAN_WIDGET_TO_WIDGET (page->leftProgressBar);
		gtk_widget_set_margin_top (widget, 2);
		gtk_widget_set_margin_bottom (widget, 2);
		gtk_widget_set_margin_start (widget, 2);
		gtk_widget_set_margin_end (widget, 8);
		gtk_widget_set_vexpand (widget, true);
		gtk_grid_attach (page->grid, widget, 0, 0, 1, 5);
	}

	page->dataLoggerPanel = GTK_GRID (gtk_grid_new ());
	gtk_grid_attach (page->grid, GTK_WIDGET (page->dataLoggerPanel), 1, 0, 2, 2);
	gtk_widget_set_valign (GTK_WIDGET (page->dataLoggerPanel), GTK_ALIGN_FILL);
	gtk_widget_set_margin_top (GTK_WIDGET (page->dataLoggerPanel), 20);

	page->dataLoggerTitle = canWidgetLoad (databases, databaseCount, jsonGetObjectV2 (config, "dataLoggerTitle"), PAGE_WIDGET_STYLE (page));
	if (page->dataLoggerTitle != NULL)
	{
		GtkWidget* widget = CAN_WIDGET_TO_WIDGET (page->dataLoggerTitle);
		gtkTryLabelSetFont (widget, page->style.dataLoggerTitleFont);
		gtkTryLabelSetColor (widget, &page->style.baseStyle->fontColor);
		gtkTryLabelSetXAlign (widget, 0);
		gtk_widget_set_margin_top (widget, 3);
		gtk_widget_set_margin_start (widget, 3);
		gtk_grid_attach (page->dataLoggerPanel, widget, 0, 0, 1, 1);
	}

	page->dataLoggerStat = canWidgetLoad (databases, databaseCount, jsonGetObjectV2 (config, "dataLoggerStat"), PAGE_WIDGET_STYLE (page));
	if (page->dataLoggerStat != NULL)
	{
		GtkWidget* widget = CAN_WIDGET_TO_WIDGET (page->dataLoggerStat);
		gtkTryLabelSetFont (widget, page->style.dataLoggerStatFont);
		gtkTryLabelSetColor (widget, &page->style.baseStyle->fontColor);
		gtkTryLabelSetXAlign (widget, 0);
		gtk_widget_set_margin_start (widget, 3);
		gtk_grid_attach (page->dataLoggerPanel, widget, 0, 1, 1, 1);
	}

	GtkWidget* padding = gtk_grid_new ();
	gtk_widget_set_size_request (padding, 80, 0);
	gtk_grid_attach (page->grid, padding, 1, 2, 1, 1);

	page->leftPanel = gtk_grid_new ();
	gtk_widget_set_margin_end (page->leftPanel, 10);
	gtk_widget_set_valign (page->leftPanel, GTK_ALIGN_CENTER);
	gtk_grid_attach (page->grid, page->leftPanel, 2, 2, 1, 1);

	cJSON* leftPanelConfig = jsonGetObjectV2 (config, "leftSidePanel");
	sidePanelLoad (page, leftPanelConfig, databases, databaseCount, &page->leftPanel, &page->leftPanelWidgets, &page->leftPanelWidgetCount);

	page->faultPanel = GTK_GRID (gtk_grid_new ());
	gtk_widget_set_margin_top (GTK_WIDGET (page->faultPanel), 20);
	gtk_widget_set_margin_bottom (GTK_WIDGET (page->faultPanel), 20);
	gtk_widget_set_margin_start (GTK_WIDGET (page->faultPanel), 40);
	gtk_grid_attach (page->grid, GTK_WIDGET (page->faultPanel), 3, 0, 2, 1);

	// Load the fault indicators

	page->faultIndicators = canWidgetLoadArray (databases, databaseCount, jsonGetObjectV2 (config, "faultIndicators"),
		PAGE_WIDGET_STYLE (page), &page->faultIndicatorCount);
	if (page->faultIndicators == NULL)
		return NULL;

	for (size_t index = 0; index < page->faultIndicatorCount; ++index)
	{
		if (page->faultIndicators [index] == NULL)
			continue;

		GtkWidget* widget = CAN_WIDGET_TO_WIDGET (page->faultIndicators [index]);
		gtk_widget_set_halign (widget, GTK_ALIGN_FILL);
		gtk_widget_set_hexpand (widget, true);
		gtk_widget_set_margin_top (widget, 5);
		gtk_widget_set_margin_bottom (widget, 5);
		gtk_widget_set_margin_start (widget, 4);
		gtk_widget_set_margin_end (widget, 4);
		gtk_grid_attach (page->faultPanel, widget, index, 0, 1, 1);
	}

	// Load the center panel

	page->centerPanelTitle = gtk_label_new ("");
	gtkLabelSetFont (GTK_LABEL (page->centerPanelTitle), page->style.centerPanelTitleFont);
	gtkLabelSetColor (GTK_LABEL (page->centerPanelTitle), &page->style.baseStyle->fontColor);
	gtk_label_set_xalign (GTK_LABEL (page->centerPanelTitle), 0);
	gtk_grid_attach (page->grid, page->centerPanelTitle, 3, 1, 1, 1);

	page->centerPanel = gtk_grid_new ();
	gtk_widget_set_hexpand (page->centerPanel, true);
	gtk_widget_set_vexpand (page->centerPanel, true);
	gtk_grid_attach (page->grid, page->centerPanel, 3, 2, 1, 1);

	cJSON* centerPanelConfig = jsonGetObjectV2 (config, "centerPanel");
	centerPanelLoad (page, centerPanelConfig, databases, databaseCount);

	page->rightPanel = gtk_grid_new ();
	gtk_widget_set_margin_start (page->rightPanel, 10);
	gtk_widget_set_valign (page->rightPanel, GTK_ALIGN_CENTER);
	gtk_grid_attach (page->grid, page->rightPanel, 4, 2, 1, 1);

	cJSON* rightPanelConfig = jsonGetObjectV2 (config, "rightSidePanel");
	sidePanelLoad (page, rightPanelConfig, databases, databaseCount, &page->rightPanel, &page->rightPanelWidgets, &page->rightPanelWidgetCount);

	page->buttonPanel = GTK_GRID (gtk_grid_new ());
	gtk_widget_set_margin_top (GTK_WIDGET (page->buttonPanel), 40);
	gtk_grid_attach (page->grid, GTK_WIDGET (page->buttonPanel), 2, 3, 3, 1);

	// Load the right progress bar

	cJSON* rightProgressBarConfig = jsonGetObjectV2 (config, "rightProgressBar");
	page->rightProgressBar = canWidgetLoad (databases, databaseCount, rightProgressBarConfig, PAGE_WIDGET_STYLE (page));
	if (page->rightProgressBar != NULL)
	{
		GtkWidget* widget = CAN_WIDGET_TO_WIDGET (page->rightProgressBar);
		gtk_widget_set_margin_top (widget, 2);
		gtk_widget_set_margin_bottom (widget, 2);
		gtk_widget_set_margin_start (widget, 5);
		gtk_widget_set_margin_end (widget, 5);
		gtk_widget_set_vexpand (widget, true);
		gtk_grid_attach (page->grid, widget, 5, 0, 1, 5);
	}

	// Return the created page (cast to the base type).
	return (page_t*) page;
}
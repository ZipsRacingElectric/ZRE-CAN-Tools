// Header
#include "page_autox.h"

// Includes
#include "gtk_util.h"
#include "can_widgets/can_label_bool.h"
#include "can_widgets/can_label_float.h"
#include "can_widgets/can_progress_bar.h"
#include "can_widgets/can_indicator.h"
#include "stylized_widgets/stylized_button.h"

const canIndicatorPoint_t FAULT_INDICATOR_POLYGON [] =
{
	{0.50, 0.00},
	{0.09, 0.00},
	{0.09, 0.02},
	{0.18, 0.40},
	{0.18, 0.67},
	{0.01, 0.02},
	{0.09, 0.02},
	{0.09, 0.00},
	{0.00, 0.00},
	{0.00, 0.50},
	{0.01, 0.45},
	{0.15, 0.98},
	{0.06, 0.98},
	{0.01, 0.80},
	{0.01, 0.45},
	{0.00, 0.45},
	{0.00, 1.00},
	{0.50, 1.00},
	{0.50, 0.98},
	{0.18, 0.98},
	{0.18, 0.02},
	{0.50, 0.02},
	{0.50, 0.06},
	{0.50, 0.94},
	{0.20, 0.94},
	{0.20, 0.06},
	{0.50, 0.06},

	{0.50, 0.06},
	{0.80, 0.06},
	{0.80, 0.94},
	{0.50, 0.94},
	{0.50, 0.06},
	{0.50, 0.02},
	{0.82, 0.02},
	{0.82, 0.98},
	{0.50, 0.98},
	{0.50, 1.00},
	{1.00, 1.00},
	{1.00, 0.45},
	{0.99, 0.45},
	{0.99, 0.80},
	{0.94, 0.98},
	{0.85, 0.98},
	{0.99, 0.45},
	{1.00, 0.50},
	{1.00, 0.00},
	{0.91, 0.00},
	{0.91, 0.02},
	{0.99, 0.02},
	{0.82, 0.67},
	{0.82, 0.40},
	{0.91, 0.02},
	{0.91, 0.00},
	{0.50, 0.00},
};
const size_t FAULT_INDICATOR_POLYGON_SIZE = sizeof (FAULT_INDICATOR_POLYGON) / sizeof (canIndicatorPoint_t);

static void drawHorizontalDecals (cairo_t* cr, pageAutox_t* page, float x0, float x1, float y, int count, bool direction, bool drawDark, bool drawLight, bool clip)
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
	pageAutox_t* page = arg;
	(void) area;
	(void) arg;

	gdk_cairo_set_source_rgba (cr, &page->style.pageStyle->backgroundColor);
	cairo_rectangle (cr, 0, 0, width, height);
	cairo_fill (cr);

	graphene_rect_t bounds;
	if (!gtk_widget_compute_bounds (GTK_WIDGET (page->grid), GTK_WIDGET (area), &bounds))
		bounds = *graphene_rect_zero ();

	float xGMin = 0;
	float yGMin = bounds.origin.y;
	float yGMax = bounds.origin.y + bounds.size.height;

	if (!gtk_widget_compute_bounds (GTK_WIDGET (page->rightPanel), GTK_WIDGET (area), &bounds))
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

	if (!gtk_widget_compute_bounds (GTK_WIDGET (page->centerTitle), GTK_WIDGET (area), &bounds))
		bounds = *graphene_rect_zero ();

	float x2 = bounds.origin.x;
	float y2 = bounds.origin.y;

	x1 -= (y2 - y1) / 2;

	if (!gtk_widget_compute_bounds (GTK_WIDGET (page->rightPanel), GTK_WIDGET (area), &bounds))
		bounds = *graphene_rect_zero ();

	float x3 = bounds.origin.x + bounds.size.width;
	float y3 = y2;

	if (!gtk_widget_compute_bounds (CAN_WIDGET_TO_WIDGET (page->centerStat), GTK_WIDGET (area), &bounds))
		bounds = *graphene_rect_zero ();

	float x4 = x3;
	float y4 = bounds.origin.y + bounds.size.height;

	if (!gtk_widget_compute_bounds (GTK_WIDGET (page->leftPanel), GTK_WIDGET (area), &bounds))
		bounds = *graphene_rect_zero ();

	float x5 = bounds.origin.x;
	float y5 = y4;

	if (!gtk_widget_compute_bounds (CAN_WIDGET_TO_WIDGET (page->bse), GTK_WIDGET (area), &bounds))
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

	gdk_cairo_set_source_rgba (cr, &page->style.pageStyle->backgroundColor);
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

	gdk_cairo_set_source_rgba (cr, &page->style.pageStyle->borderColor);
	cairo_set_line_width (cr, page->style.pageStyle->borderThickness);

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

	if (!gtk_widget_compute_bounds (GTK_WIDGET (page->leftPanel), GTK_WIDGET (area), &bounds))
		bounds = *graphene_rect_zero ();
	cairo_rectangle (cr, bounds.origin.x, bounds.origin.y, bounds.size.width, bounds.size.height);
	cairo_stroke (cr);

	if (!gtk_widget_compute_bounds (GTK_WIDGET (page->rightPanel), GTK_WIDGET (area), &bounds))
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

page_t* pageAutoxInit (canDatabase_t* database, pageStyle_t* style)
{
	// Allocate the page
	pageAutox_t* page = malloc (sizeof (pageAutox_t));
	if (page == NULL)
		return NULL;

	// Setup the VMT
	page->vmt = (pageVmt_t)
	{
		.update			= pageAutoxUpdate,
		.appendButton	= pageAutoxAppendButton,
		.widget			= gtk_overlay_new ()
	};

	page->style = (pageAutoxStyle_t)
	{
		.pageStyle				= style,

		.appsColor				= gdkHexToColor ("#51BC58"),
		.bseColor				= gdkHexToColor ("#FF0000"),

		.decalTickColorLight	= gdkHexToColor ("#51BC58"),
		.decalTickColorDark		= gdkHexToColor ("#000000"),
		.decalUpperTickMargin	= 20,
		.decalUpperTickSpacing	= 50,
		.decalCenterTickMargin	= 10,
		.decalCenterTickSpacing	= 80,
		.decalHeight			= 6,
		.decalThickness			= 2,
		.decalWidth				= 6,

		.faultActiveColor		= gdkHexToColor ("#FF0000"),
		.faultInactiveColor		= gdkHexToColor ("#580000"),

		.gradientStartColor		= gdkHexToColor ("#B63D24"),
		.gradientStartPosition	= 0.0f,
		.gradientMiddleColor	= gdkHexToColor ("#C79418"),
		.gradientMiddlePosition	= 0.4f,
		.gradientEndColor		= gdkHexToColor ("#0D5E2C"),
		.gradientEndPosition	= 1.0f,

		.dataLoggerTitleFont	= "Futura Std Bold Condensed 36px",
		.dataLoggerStatFont		= "ITC Avant Garde Gothic CE Book 22px",
		.centerPanelTitleFont	= "ITC Avant Garde Gothic CE Book 22px",
		.centerPanelStatFont	= "Technology Bold 200px",
		.leftPanelTitleFont		= "Futura Std Bold Condensed 26px",
		.leftPanelStatFont		= "ITC Avant Garde Gothic CE Book 26px",
		.rightPanelTitleFont	= "Futura Std Bold Condensed 26px",
		.rightPanelStatFont		= "ITC Avant Garde Gothic CE Book 26px",
		.faultIndicatorFont		= "Futura Std Bold Condensed 32px"
	};

	page->buttonCount = 0;

	GtkWidget* bg = gtk_drawing_area_new ();
	gtk_drawing_area_set_draw_func (GTK_DRAWING_AREA (bg), drawBg, page, NULL);
	gtk_overlay_set_child (GTK_OVERLAY (page->vmt.widget), bg);

	page->grid = GTK_GRID (gtk_grid_new ());
	gtk_widget_set_margin_top (GTK_WIDGET (page->grid), 10);
	gtk_widget_set_margin_bottom (GTK_WIDGET (page->grid), 10);
	gtk_overlay_add_overlay (GTK_OVERLAY (page->vmt.widget), GTK_WIDGET (page->grid));

	// BSE bar
	page->bse = canProgressBarInit (database, &(canProgressBarConfig_t)
	{
		.barConfig	=
		{
			.width				= 10,
			.height				= 0,
			.borderThickness	= page->style.pageStyle->borderThickness,
			.orientation		= GTK_ORIENTATION_VERTICAL,
			.inverted			= false,
			.backgroundColor	= page->style.pageStyle->backgroundColor,
			.borderColor		= page->style.pageStyle->backgroundColor,
			.fillColor			= page->style.bseColor
		},
		.signalName 			= "BSE_FRONT_PERCENT",
		.min					= 0,
		.max					= 100
	});
	gtk_widget_set_margin_top (CAN_WIDGET_TO_WIDGET (page->bse), 2);
	gtk_widget_set_margin_bottom (CAN_WIDGET_TO_WIDGET (page->bse), 2);
	gtk_widget_set_margin_start (CAN_WIDGET_TO_WIDGET (page->bse), 2);
	gtk_widget_set_margin_end (CAN_WIDGET_TO_WIDGET (page->bse), 8);
	gtk_widget_set_vexpand (CAN_WIDGET_TO_WIDGET (page->bse), true);
	gtk_grid_attach (page->grid, CAN_WIDGET_TO_WIDGET (page->bse), 0, 0, 1, 5);

	page->dataLoggerPanel = GTK_GRID (gtk_grid_new ());
	gtk_grid_attach (page->grid, GTK_WIDGET (page->dataLoggerPanel), 1, 0, 2, 2);
	gtk_widget_set_valign (GTK_WIDGET (page->dataLoggerPanel), GTK_ALIGN_FILL);
	gtk_widget_set_margin_top (GTK_WIDGET (page->dataLoggerPanel), 20);

	page->dataLoggerTitle = canLabelBoolInit (database, &(canLabelBoolConfig_t)
	{
		.signalName		= "SESSION_NUMBER",
		.activeValue	= "LOGGING: ON",
		.inactiveValue	= "LOGGING: ON",
		.invalidValue	= "LOGGING: OFF",
		.threshold		= 0,
		.inverted		= false
	});
	gtkLabelSetFont (CAN_LABEL_BOOL_TO_LABEL (page->dataLoggerTitle), page->style.dataLoggerTitleFont);
	gtkLabelSetColor (CAN_LABEL_BOOL_TO_LABEL (page->dataLoggerTitle), &page->style.pageStyle->fontColor);
	gtk_label_set_xalign (CAN_LABEL_BOOL_TO_LABEL (page->dataLoggerTitle), 0);
	gtk_widget_set_margin_top (CAN_WIDGET_TO_WIDGET (page->dataLoggerTitle), 3);
	gtk_widget_set_margin_start (CAN_WIDGET_TO_WIDGET (page->dataLoggerTitle), 3);
	gtk_grid_attach (page->dataLoggerPanel, CAN_WIDGET_TO_WIDGET (page->dataLoggerTitle), 0, 0, 1, 1);

	page->dataLoggerStat = canLabelFloatInit (database, &(canLabelFloatConfig_t)
	{
		.signalName		= "SESSION_NUMBER",
		.formatValue	= "Session\nNo. %.0f",
		.formatInvalid	= "Session\nNo. %s",
	});
	gtkLabelSetFont (CAN_LABEL_FLOAT_TO_LABEL (page->dataLoggerStat), page->style.dataLoggerStatFont);
	gtkLabelSetColor (CAN_LABEL_BOOL_TO_LABEL (page->dataLoggerStat), &page->style.pageStyle->fontColor);
	gtk_label_set_xalign (CAN_LABEL_FLOAT_TO_LABEL (page->dataLoggerStat), 0);
	gtk_widget_set_margin_start (CAN_WIDGET_TO_WIDGET (page->dataLoggerStat), 3);
	gtk_grid_attach (page->dataLoggerPanel, CAN_WIDGET_TO_WIDGET (page->dataLoggerStat), 0, 1, 1, 1);

	GtkWidget* padding = gtk_grid_new ();
	gtk_widget_set_size_request (padding, 80, 0);
	gtk_grid_attach (page->grid, padding, 1, 2, 1, 1);

	page->leftPanel = GTK_GRID (gtk_grid_new ());
	gtk_widget_set_margin_end (GTK_WIDGET (page->leftPanel), 10);
	gtk_widget_set_valign (GTK_WIDGET (page->leftPanel), GTK_ALIGN_CENTER);
	gtk_grid_attach (page->grid, GTK_WIDGET (page->leftPanel), 2, 2, 1, 1);

	GtkWidget* label = gtk_label_new ("Torque Config");
	gtkLabelSetFont (GTK_LABEL (label), page->style.leftPanelTitleFont);
	gtkLabelSetColor (GTK_LABEL (label), &page->style.pageStyle->fontColor);
	gtk_widget_set_size_request (label, 180, 0);
	gtk_label_set_xalign (GTK_LABEL (label), 0.5);
	gtk_widget_set_margin_top (label, 3);
	gtk_grid_attach (page->leftPanel, label, 0, 0, 2, 1);

	label = gtk_label_new ("Driving:");
	gtkLabelSetFont (GTK_LABEL (label), page->style.leftPanelStatFont);
	gtkLabelSetColor (GTK_LABEL (label), &page->style.pageStyle->fontColor);
	gtk_widget_set_halign (label, GTK_ALIGN_START);
	gtk_widget_set_margin_start (label, 3);
	gtk_grid_attach (page->leftPanel, label, 0, 1, 1, 1);

	page->drivingTorque = canLabelFloatInit (database, &(canLabelFloatConfig_t)
	{
		.signalName		= "DRIVING_TORQUE_LIMIT",
		.formatValue	= "%.0f%s",
		.formatInvalid	= "%s%s"
	});
	gtkLabelSetFont (CAN_LABEL_FLOAT_TO_LABEL (page->drivingTorque), page->style.leftPanelStatFont);
	gtkLabelSetColor (CAN_LABEL_BOOL_TO_LABEL (page->drivingTorque), &page->style.pageStyle->fontColor);
	gtk_widget_set_halign (CAN_WIDGET_TO_WIDGET (page->drivingTorque), GTK_ALIGN_END);
	gtk_widget_set_margin_end (CAN_WIDGET_TO_WIDGET (page->drivingTorque), 3);
	gtk_grid_attach (page->leftPanel, CAN_WIDGET_TO_WIDGET (page->drivingTorque), 1, 1, 1, 1);

	label = gtk_label_new ("Regen:");
	gtkLabelSetFont (GTK_LABEL (label), page->style.leftPanelStatFont);
	gtkLabelSetColor (GTK_LABEL (label), &page->style.pageStyle->fontColor);
	gtk_widget_set_halign (label, GTK_ALIGN_START);
	gtk_widget_set_margin_start (label, 3);
	gtk_grid_attach (page->leftPanel, label, 0, 2, 1, 1);

	page->regenTorque = canLabelFloatInit (database, &(canLabelFloatConfig_t)
	{
		.signalName		= "REGEN_TORQUE_LIMIT",
		.formatValue	= "%.0f%s",
		.formatInvalid	= "%s%s"
	});
	gtkLabelSetFont (CAN_LABEL_FLOAT_TO_LABEL (page->regenTorque), page->style.leftPanelStatFont);
	gtkLabelSetColor (CAN_LABEL_BOOL_TO_LABEL (page->regenTorque), &page->style.pageStyle->fontColor);
	gtk_widget_set_halign (CAN_WIDGET_TO_WIDGET (page->regenTorque), GTK_ALIGN_END);
	gtk_widget_set_margin_end (CAN_WIDGET_TO_WIDGET (page->regenTorque), 3);
	gtk_grid_attach (page->leftPanel, CAN_WIDGET_TO_WIDGET (page->regenTorque), 1, 2, 1, 1);

	label = gtk_label_new ("Index:");
	gtkLabelSetFont (GTK_LABEL (label), page->style.leftPanelStatFont);
	gtkLabelSetColor (GTK_LABEL (label), &page->style.pageStyle->fontColor);
	gtk_widget_set_halign (label, GTK_ALIGN_START);
	gtk_widget_set_margin_start (label, 3);
	gtk_grid_attach (page->leftPanel, label, 0, 3, 1, 1);

	page->torqueIndex = canLabelFloatInit (database, &(canLabelFloatConfig_t)
	{
		.signalName		= "TORQUE_ALGORITHM_INDEX",
		.formatValue	= "%.0f",
		.formatInvalid	= "%s"
	});
	gtkLabelSetFont (CAN_LABEL_FLOAT_TO_LABEL (page->torqueIndex), page->style.leftPanelStatFont);
	gtkLabelSetColor (CAN_LABEL_BOOL_TO_LABEL (page->torqueIndex), &page->style.pageStyle->fontColor);
	gtk_widget_set_halign (CAN_WIDGET_TO_WIDGET (page->torqueIndex), GTK_ALIGN_END);
	gtk_widget_set_margin_end (CAN_WIDGET_TO_WIDGET (page->torqueIndex), 3);
	gtk_grid_attach (page->leftPanel, CAN_WIDGET_TO_WIDGET (page->torqueIndex), 1, 3, 1, 1);

	label = gtk_label_new ("DRS:");
	gtkLabelSetFont (GTK_LABEL (label), page->style.leftPanelStatFont);
	gtkLabelSetColor (GTK_LABEL (label), &page->style.pageStyle->fontColor);
	gtk_widget_set_halign (label, GTK_ALIGN_START);
	gtk_widget_set_margin_start (label, 3);
	gtk_grid_attach (page->leftPanel, label, 0, 4, 1, 1);

	page->drsStatus = canIndicatorInit (database, &(canIndicatorConfig_t)
	{
		.signalName 	= "TODO",
		.threshold		= 0.5f,
		.inverted		= false,
		.width			= 26,
		.height			= 26,
		.activeColor	= gdkHexToColor ("#FFFFFF"),
		.inactiveColor	= gdkHexToColor ("#580000"),
		.invalidColor	= gdkHexToColor ("#FF0000"),
		.polygon		= NULL,
		.polygonSize	= 0
	});
	gtk_widget_set_halign (CAN_WIDGET_TO_WIDGET (page->drsStatus), GTK_ALIGN_END);
	gtk_widget_set_margin_end (CAN_WIDGET_TO_WIDGET (page->drsStatus), 3);
	gtk_grid_attach (page->leftPanel, CAN_WIDGET_TO_WIDGET (page->drsStatus), 1, 4, 1, 1);

	page->faultPanel = GTK_GRID (gtk_grid_new ());
	gtk_widget_set_margin_top (GTK_WIDGET (page->faultPanel), 20);
	gtk_widget_set_margin_bottom (GTK_WIDGET (page->faultPanel), 20);
	gtk_widget_set_margin_start (GTK_WIDGET (page->faultPanel), 40);
	gtk_grid_attach (page->grid, GTK_WIDGET (page->faultPanel), 3, 0, 2, 1);

	page->vcuFault = canIndicatorInit (database, &(canIndicatorConfig_t)
	{
		.signalName		= "VCU_FAULT",
		.threshold		= 0.5f,
		.inverted		= false,
		.width			= 100,
		.height			= 42,
		.inactiveColor	= page->style.faultInactiveColor,
		.activeColor	= page->style.faultActiveColor,
		.invalidColor	= page->style.faultActiveColor,
		.polygon		= FAULT_INDICATOR_POLYGON,
		.polygonSize	= FAULT_INDICATOR_POLYGON_SIZE,
	});
	gtk_widget_set_halign (CAN_WIDGET_TO_WIDGET (page->vcuFault), GTK_ALIGN_FILL);
	gtk_widget_set_hexpand (CAN_WIDGET_TO_WIDGET (page->vcuFault), true);
	gtk_widget_set_margin_top (CAN_WIDGET_TO_WIDGET (page->vcuFault), 5);
	gtk_widget_set_margin_bottom (CAN_WIDGET_TO_WIDGET (page->vcuFault), 5);
	gtk_widget_set_margin_start (CAN_WIDGET_TO_WIDGET (page->vcuFault), 4);
	gtk_widget_set_margin_end (CAN_WIDGET_TO_WIDGET (page->vcuFault), 4);
	gtk_grid_attach (page->faultPanel, CAN_WIDGET_TO_WIDGET (page->vcuFault), 0, 0, 1, 1);

	label = gtk_label_new ("VCU");
	gtkLabelSetFont (GTK_LABEL (label), page->style.faultIndicatorFont);
	gtkLabelSetColor (GTK_LABEL (label), &style->backgroundColor);
	gtk_grid_attach (page->faultPanel, label, 0, 0, 1, 1);

	page->bmsFault = canIndicatorInit (database, &(canIndicatorConfig_t)
	{
		.signalName		= "BMS_FAULT",
		.threshold		= 0.5f,
		.inverted		= false,
		.width			= 100,
		.height			= 42,
		.inactiveColor	= page->style.faultInactiveColor,
		.activeColor	= page->style.faultActiveColor,
		.invalidColor	= page->style.faultActiveColor,
		.polygon		= FAULT_INDICATOR_POLYGON,
		.polygonSize	= FAULT_INDICATOR_POLYGON_SIZE,
	});
	gtk_widget_set_halign (CAN_WIDGET_TO_WIDGET (page->bmsFault), GTK_ALIGN_FILL);
	gtk_widget_set_hexpand (CAN_WIDGET_TO_WIDGET (page->bmsFault), true);
	gtk_widget_set_margin_start (CAN_WIDGET_TO_WIDGET (page->bmsFault), 4);
	gtk_widget_set_margin_end (CAN_WIDGET_TO_WIDGET (page->bmsFault), 4);
	gtk_grid_attach (page->faultPanel, CAN_WIDGET_TO_WIDGET (page->bmsFault), 1, 0, 1, 1);

	label = gtk_label_new ("BMS");
	gtkLabelSetFont (GTK_LABEL (label), page->style.faultIndicatorFont);
	gtkLabelSetColor (GTK_LABEL (label), &style->backgroundColor);
	gtk_grid_attach (page->faultPanel, label, 1, 0, 1, 1);

	page->amkFault = canIndicatorInit (database, &(canIndicatorConfig_t)
	{
		.signalName		= "AMK_FAULT",
		.threshold		= 0.5f,
		.inverted		= false,
		.width			= 100,
		.height			= 42,
		.inactiveColor	= page->style.faultInactiveColor,
		.activeColor	= page->style.faultActiveColor,
		.invalidColor	= page->style.faultActiveColor,
		.polygon		= FAULT_INDICATOR_POLYGON,
		.polygonSize	= FAULT_INDICATOR_POLYGON_SIZE,
	});
	gtk_widget_set_halign (CAN_WIDGET_TO_WIDGET (page->amkFault), GTK_ALIGN_FILL);
	gtk_widget_set_hexpand (CAN_WIDGET_TO_WIDGET (page->amkFault), true);
	gtk_widget_set_margin_start (CAN_WIDGET_TO_WIDGET (page->amkFault), 4);
	gtk_widget_set_margin_end (CAN_WIDGET_TO_WIDGET (page->amkFault), 4);
	gtk_grid_attach (page->faultPanel, CAN_WIDGET_TO_WIDGET (page->amkFault), 2, 0, 1, 1);

	label = gtk_label_new ("AMK");
	gtkLabelSetFont (GTK_LABEL (label), page->style.faultIndicatorFont);
	gtkLabelSetColor (GTK_LABEL (label), &style->backgroundColor);
	gtk_grid_attach (page->faultPanel, label, 2, 0, 1, 1);

	page->gpsFault = canIndicatorInit (database, &(canIndicatorConfig_t)
	{
		.signalName		= "GPS_STATUS",
		.threshold		= 2.5f,
		.inverted		= true,
		.width			= 100,
		.height			= 42,
		.inactiveColor	= page->style.faultInactiveColor,
		.activeColor	= page->style.faultActiveColor,
		.invalidColor	= page->style.faultActiveColor,
		.polygon		= FAULT_INDICATOR_POLYGON,
		.polygonSize	= FAULT_INDICATOR_POLYGON_SIZE,
	});
	gtk_widget_set_halign (CAN_WIDGET_TO_WIDGET (page->gpsFault), GTK_ALIGN_FILL);
	gtk_widget_set_hexpand (CAN_WIDGET_TO_WIDGET (page->gpsFault), true);
	gtk_widget_set_margin_start (CAN_WIDGET_TO_WIDGET (page->gpsFault), 4);
	gtk_widget_set_margin_end (CAN_WIDGET_TO_WIDGET (page->gpsFault), 4);
	gtk_grid_attach (page->faultPanel, CAN_WIDGET_TO_WIDGET (page->gpsFault), 3, 0, 1, 1);

	label = gtk_label_new ("GPS");
	gtkLabelSetFont (GTK_LABEL (label), page->style.faultIndicatorFont);
	gtkLabelSetColor (GTK_LABEL (label), &style->backgroundColor);
	gtk_grid_attach (page->faultPanel, label, 3, 0, 1, 1);

	page->centerTitle = gtk_label_new ("Vehicle Speed (Km/h):");
	gtkLabelSetFont (GTK_LABEL (page->centerTitle), page->style.centerPanelTitleFont);
	gtkLabelSetColor (GTK_LABEL (page->centerTitle), &page->style.pageStyle->fontColor);
	gtk_label_set_xalign (GTK_LABEL (page->centerTitle), 0);
	gtk_grid_attach (page->grid, page->centerTitle, 3, 1, 1, 1);

	page->centerStat = canLabelFloatInit (database, &(canLabelFloatConfig_t)
	{
		.signalName		= "SPEED",
		.formatValue	= "%02.0f",
		.formatInvalid	= "XX"
	});
	gtkLabelSetFont (CAN_LABEL_FLOAT_TO_LABEL (page->centerStat), page->style.centerPanelStatFont);
	gtkLabelSetColor (CAN_LABEL_BOOL_TO_LABEL (page->centerStat), &page->style.pageStyle->fontColor);
	gtk_widget_set_hexpand (CAN_WIDGET_TO_WIDGET (page->centerStat), true);
	gtk_widget_set_vexpand (CAN_WIDGET_TO_WIDGET (page->centerStat), true);
	gtk_grid_attach (page->grid, CAN_WIDGET_TO_WIDGET (page->centerStat), 3, 2, 1, 1);

	page->rightPanel = GTK_GRID (gtk_grid_new ());
	gtk_widget_set_margin_start (GTK_WIDGET (page->rightPanel), 10);
	gtk_widget_set_valign (GTK_WIDGET (page->rightPanel), GTK_ALIGN_CENTER);
	gtk_grid_attach (page->grid, GTK_WIDGET (page->rightPanel), 4, 2, 1, 1);

	label = gtk_label_new ("Powertrain");
	gtkLabelSetFont (GTK_LABEL (label), page->style.rightPanelTitleFont);
	gtkLabelSetColor (GTK_LABEL (label), &page->style.pageStyle->fontColor);
	gtk_widget_set_size_request (label, 180, 0);
	gtk_label_set_xalign (GTK_LABEL (label), 0.5);
	gtk_widget_set_margin_top (label, 3);
	gtk_grid_attach (page->rightPanel, label, 0, 0, 2, 1);

	label = gtk_label_new ("GLV:");
	gtkLabelSetFont (GTK_LABEL (label), page->style.rightPanelStatFont);
	gtkLabelSetColor (GTK_LABEL (label), &page->style.pageStyle->fontColor);
	gtk_widget_set_halign (label, GTK_ALIGN_START);
	gtk_widget_set_margin_start (label, 3);
	gtk_grid_attach (page->rightPanel, label, 0, 1, 1, 1);

	page->glvVoltage = canLabelFloatInit (database, &(canLabelFloatConfig_t)
	{
		.signalName		= "GLV_BATTERY_VOLTAGE",
		.formatValue	= "%.1f %s",
		.formatInvalid	= "%s %s"
	});
	gtkLabelSetFont (CAN_LABEL_FLOAT_TO_LABEL (page->glvVoltage), page->style.rightPanelStatFont);
	gtkLabelSetColor (CAN_LABEL_BOOL_TO_LABEL (page->glvVoltage), &page->style.pageStyle->fontColor);
	gtk_widget_set_halign (CAN_WIDGET_TO_WIDGET (page->glvVoltage), GTK_ALIGN_END);
	gtk_widget_set_margin_end (CAN_WIDGET_TO_WIDGET (page->glvVoltage), 3);
	gtk_grid_attach (page->rightPanel, CAN_WIDGET_TO_WIDGET (page->glvVoltage), 1, 1, 1, 1);

	label = gtk_label_new ("HV:");
	gtkLabelSetFont (GTK_LABEL (label), page->style.rightPanelStatFont);
	gtkLabelSetColor (GTK_LABEL (label), &page->style.pageStyle->fontColor);
	gtk_widget_set_halign (label, GTK_ALIGN_START);
	gtk_widget_set_margin_start (label, 3);
	gtk_grid_attach (page->rightPanel, label, 0, 2, 1, 1);

	page->hvVoltage = canLabelFloatInit (database, &(canLabelFloatConfig_t)
	{
		.signalName		= "PACK_VOLTAGE",
		.formatValue	= "%.1f %s",
		.formatInvalid	= "%s %s"
	});
	gtkLabelSetFont (CAN_LABEL_FLOAT_TO_LABEL (page->hvVoltage), page->style.rightPanelStatFont);
	gtkLabelSetColor (CAN_LABEL_BOOL_TO_LABEL (page->hvVoltage), &page->style.pageStyle->fontColor);
	gtk_widget_set_halign (CAN_WIDGET_TO_WIDGET (page->hvVoltage), GTK_ALIGN_END);
	gtk_widget_set_margin_end (CAN_WIDGET_TO_WIDGET (page->hvVoltage), 3);
	gtk_grid_attach (page->rightPanel, CAN_WIDGET_TO_WIDGET (page->hvVoltage), 1, 2, 1, 1);

	label = gtk_label_new ("Inv:");
	gtkLabelSetFont (GTK_LABEL (label), page->style.rightPanelStatFont);
	gtkLabelSetColor (GTK_LABEL (label), &page->style.pageStyle->fontColor);
	gtk_widget_set_halign (label, GTK_ALIGN_START);
	gtk_widget_set_margin_start (label, 3);
	gtk_grid_attach (page->rightPanel, label, 0, 3, 1, 1);

	page->inverterMaxTemp = canLabelFloatInit (database, &(canLabelFloatConfig_t)
	{
		.signalName		= "AMK_INVERTER_TEMPERATURE_MAX",
		.formatValue	= "%.1f %s",
		.formatInvalid	= "%s %s"
	});
	gtkLabelSetFont (CAN_LABEL_FLOAT_TO_LABEL (page->inverterMaxTemp), page->style.rightPanelStatFont);
	gtkLabelSetColor (CAN_LABEL_BOOL_TO_LABEL (page->inverterMaxTemp), &page->style.pageStyle->fontColor);
	gtk_widget_set_halign (CAN_WIDGET_TO_WIDGET (page->inverterMaxTemp), GTK_ALIGN_END);
	gtk_widget_set_margin_end (CAN_WIDGET_TO_WIDGET (page->inverterMaxTemp), 3);
	gtk_grid_attach (page->rightPanel, CAN_WIDGET_TO_WIDGET (page->inverterMaxTemp), 1, 3, 1, 1);

	label = gtk_label_new ("Mtr:");
	gtkLabelSetFont (GTK_LABEL (label), page->style.rightPanelStatFont);
	gtkLabelSetColor (GTK_LABEL (label), &page->style.pageStyle->fontColor);
	gtk_widget_set_halign (label, GTK_ALIGN_START);
	gtk_widget_set_margin_start (label, 3);
	gtk_grid_attach (page->rightPanel, label, 0, 4, 1, 1);

	page->motorMaxTemp = canLabelFloatInit (database, &(canLabelFloatConfig_t)
	{
		.signalName		= "AMK_MOTOR_TEMPERATURE_MAX",
		.formatValue	= "%.1f %s",
		.formatInvalid	= "%s %s"
	});
	gtkLabelSetFont (CAN_LABEL_FLOAT_TO_LABEL (page->motorMaxTemp), page->style.rightPanelStatFont);
	gtkLabelSetColor (CAN_LABEL_BOOL_TO_LABEL (page->motorMaxTemp), &page->style.pageStyle->fontColor);
	gtk_widget_set_halign (CAN_WIDGET_TO_WIDGET (page->motorMaxTemp), GTK_ALIGN_END);
	gtk_widget_set_margin_end (CAN_WIDGET_TO_WIDGET (page->motorMaxTemp), 3);
	gtk_grid_attach (page->rightPanel, CAN_WIDGET_TO_WIDGET (page->motorMaxTemp), 1, 4, 1, 1);

	page->buttonPanel = GTK_GRID (gtk_grid_new ());
	gtk_widget_set_margin_top (GTK_WIDGET (page->buttonPanel), 40);
	gtk_grid_attach (page->grid, GTK_WIDGET (page->buttonPanel), 2, 3, 3, 1);

	page->apps = canProgressBarInit (database, &(canProgressBarConfig_t)
	{
		.barConfig	=
		{
			.width				= 10,
			.height				= 0,
			.borderThickness	= page->style.pageStyle->borderThickness,
			.orientation		= GTK_ORIENTATION_VERTICAL,
			.inverted			= false,
			.backgroundColor	= page->style.pageStyle->backgroundColor,
			.borderColor		= page->style.appsColor,
			.fillColor			= page->style.appsColor
		},
		.signalName 			= "APPS_1_PERCENT",
		.min					= 0,
		.max					= 100
	});
	gtk_widget_set_margin_top (CAN_WIDGET_TO_WIDGET (page->apps), 2);
	gtk_widget_set_margin_bottom (CAN_WIDGET_TO_WIDGET (page->apps), 2);
	gtk_widget_set_margin_start (CAN_WIDGET_TO_WIDGET (page->apps), 5);
	gtk_widget_set_margin_end (CAN_WIDGET_TO_WIDGET (page->apps), 5);
	gtk_widget_set_vexpand (CAN_WIDGET_TO_WIDGET (page->apps), true);
	gtk_grid_attach (page->grid, CAN_WIDGET_TO_WIDGET (page->apps), 5, 0, 1, 5);

	// Return the created page (cast to the base type).
	return (page_t*) page;
}

void pageAutoxAppendButton (void* pageArg, const char* label, pageButtonCallback_t* callback, void* arg, bool currentPage)
{
	pageAutox_t* page = pageArg;

	stylizedButton_t* button = stylizedButtonInit (callback, arg, &(stylizedButtonConfig_t)
	{
		.width				= 0,
		.height				= page->style.pageStyle->buttonHeight,
		.label				= label,
		.borderThickness	= page->style.pageStyle->borderThickness,
		.backgroundColor	= page->style.pageStyle->backgroundColor,
		.borderColor		= page->style.pageStyle->borderColor,
		.selectedColor		= page->style.pageStyle->fontColor,
		.indicatorColor		= currentPage ?
			page->style.pageStyle->indicatorActiveColor : page->style.pageStyle->indicatorInactiveColor
	});
	gtkLabelSetFont (STYLIZED_BUTTON_TO_LABEL (button), page->style.pageStyle->buttonFont);
	gtk_widget_set_margin_top (STYLIZED_BUTTON_TO_WIDGET (button), 8);
	gtk_widget_set_margin_start (STYLIZED_BUTTON_TO_WIDGET (button), 4);
	gtk_widget_set_margin_end (STYLIZED_BUTTON_TO_WIDGET (button), 4);
	gtk_widget_set_hexpand (STYLIZED_BUTTON_TO_WIDGET (button), true);
	gtk_grid_attach (GTK_GRID (page->buttonPanel), STYLIZED_BUTTON_TO_WIDGET (button), page->buttonCount, 0, 1, 1);
	++page->buttonCount;
}

void pageAutoxUpdate (void* pageArg)
{
	pageAutox_t* page = pageArg;

	canWidgetUpdate (page->bse);
	canWidgetUpdate (page->apps);
	canWidgetUpdate (page->dataLoggerTitle);
	canWidgetUpdate (page->dataLoggerStat);
	canWidgetUpdate (page->glvVoltage);
	canWidgetUpdate (page->hvVoltage);
	canWidgetUpdate (page->inverterMaxTemp);
	canWidgetUpdate (page->motorMaxTemp);
	canWidgetUpdate (page->drivingTorque);
	canWidgetUpdate (page->regenTorque);
	canWidgetUpdate (page->torqueIndex);
	canWidgetUpdate (page->centerStat);
	canWidgetUpdate (page->drsStatus);
	canWidgetUpdate (page->vcuFault);
	canWidgetUpdate (page->bmsFault);
	canWidgetUpdate (page->amkFault);
	canWidgetUpdate (page->gpsFault);
}
#include "can_label_timer.h"
#include "../gtk_util.h"
#include "cjson/cjson_util.h"

typedef struct
{
	canWidgetVmt_t vmt;
	canLabelTimerConfig_t config;
	canDatabase_t* database;
} canLabelTimer_t;

static void draw (GtkDrawingArea* area, cairo_t* cr, int width, int height, gpointer arg)
{
	(void) area;

	// TODO(DiBacco): load styles from config
	const char* BG = "#000000";
	GdkRGBA bg = gdkHexToColor (BG);
	gdk_cairo_set_source_rgba (cr, &bg);

	// Creates the background of the timer
	cairo_move_to (cr, 0, 0);
	cairo_rectangle (cr, 0, 0, width, height);
	cairo_fill (cr);
}


static void update (void* widget)
{
	// TODO(DiBacco): look into the g_signal_connect function
	// TODO(DiBacco): consider moving widgets into the same scope as .config and .widget

	float value;
	canLabelTimer_t* timer = widget;
	canDatabaseGetFloat(timer->database, timer->config.signalIndex, &value);

	// TODO(DiBacco): change condition to value > 0.
	if (value && !timer->config.running)
	{
		timer->config.running = true;
		clock_gettime (CLOCK_REALTIME, &timer->config.startTime);
	}

	clock_gettime (CLOCK_REALTIME, &timer->config.currentTime);
	struct timespec delta = timespecSub (&timer->config.currentTime, &timer->config.startTime);

	char* time = malloc(sizeof (char));
	sprintf(time, "%02ld:%02ld:%03ld",
		delta.tv_sec / 60,
		delta.tv_sec % 60,
		delta.tv_nsec / 1000000
	);

	gtk_label_set_text (GTK_LABEL (timer->config.timer), time);
	free (time);
}

static void styleLoad (canLabelTimerStyle_t* style, pageStyle_t* baseStyle, cJSON* config)
{
	*style = (canLabelTimerStyle_t)
	{
		.baseStyle = baseStyle
	};

	if (config == NULL)
		return;

	style->baseStyle = pageStyleLoad (jsonGetObjectV2 (config, "baseStyle"), baseStyle);

	// TODO(DiBacco): after hardcoding timer, create json config for the timer widget

}

canWidget_t* canLabelTimerInit (canDatabase_t* database, canLabelTimerConfig_t* config)
{
    // Allocate the object
	canLabelTimer_t* timer = malloc (sizeof (canLabelTimer_t));
	if (timer == NULL)
		return NULL;

    // Init the object
	*timer = (canLabelTimer_t)
	{
		.vmt =
		{
			.update = update,
			.widget	= gtk_overlay_new (),
		},
		.config		= *config,
		.database	= database,
	};

	timer->config.timer = gtk_label_new ("00:00:000");

	PangoAttrList* attrList = pango_attr_list_new ();
	PangoAttribute* fontSize = pango_attr_size_new (20 * PANGO_SCALE);
	pango_attr_list_insert (attrList, fontSize);

	gtk_label_set_attributes (GTK_LABEL (timer->config.timer), attrList);
	pango_attr_list_unref (attrList);

	// TODO(DiBacco): load styles from config
	const char* TEXT_COLOR = "#ffffff";
	GdkRGBA textColor = gdkHexToColor(TEXT_COLOR);
	gtkLabelSetColor (GTK_LABEL (timer->config.timer), &textColor);

	timer->config.area = gtk_drawing_area_new ();

	gtk_overlay_set_child (GTK_OVERLAY (CAN_WIDGET_TO_WIDGET (timer)), timer->config.area);
	gtk_overlay_add_overlay (GTK_OVERLAY (CAN_WIDGET_TO_WIDGET (timer)), timer->config.timer);

	// TODO(DiBacco): modify the drawing area height and width?
	gtk_drawing_area_set_content_width (GTK_DRAWING_AREA (timer->config.area), config->width);
	gtk_drawing_area_set_content_height (GTK_DRAWING_AREA (timer->config.area), config->height);
	gtk_drawing_area_set_draw_func (GTK_DRAWING_AREA (timer->config.area), draw, timer, NULL);

	// TODO(DiBacco): load styles in via cJson.

	timer->config.running = false;
	timer->config.signalIndex = canDatabaseFindSignal (database, "WHEEL_BUTTON_TOP_LEFT");

    // Update initial value
	update (timer);

	// Cast into the base type
	return (canWidget_t*) timer;
}

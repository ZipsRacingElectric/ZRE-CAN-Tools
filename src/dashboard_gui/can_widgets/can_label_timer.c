#include "can_label_timer.h"
#include "../gtk_util.h"
#include "cjson/cjson_util.h"
#include "debug.h"

static void draw (GtkDrawingArea* area, cairo_t* cr, int width, int height, gpointer arg)
{
	(void) area;
	(void) arg;

	canLabelTimer_t* timer = arg;

	// Creates the background of the timer
	gdk_cairo_set_source_rgba (cr, &timer->config.backgroundColor);
	cairo_rectangle (cr, 0, 0, width, height);
	cairo_fill_preserve (cr);

	// Creates the border of the timer
	cairo_set_line_width (cr, timer->config.borderThickness);
    gdk_cairo_set_source_rgba (cr, &timer->config.borderColor);
    cairo_stroke(cr);
}

static void setText (canLabelTimer_t* timer, struct timespec* time)
{
	sprintf (timer->formattedTime, "%02lu:%02lu:%03lu",
		(unsigned long) (time->tv_sec / 60),
		(unsigned long) (time->tv_sec % 60),
		(unsigned long) (time->tv_nsec / 1000000)
	);

	gtk_label_set_text (GTK_LABEL (timer->timer), timer->formattedTime);
}

static void update (void* widget)
{
	float value = 0;
	canLabelTimer_t* timer = widget;

	if (canDatabaseGetFloat (timer->database, timer->signalIndex, &value) != 0)
	{
		// Indicates a can database timeout state
		gtk_label_set_text (GTK_LABEL (timer->timer), "--:--:---");
		return;
	}

	if ((value >= timer->config.threshold) != timer->buttonPressed)
	{
		if (timer->running)
		{
			clock_gettime (CLOCK_MONOTONIC, &timer->currentTime);
	 		struct timespec delta = timespecSub (&timer->currentTime, &timer->startTime);

			if (timer->mode == LAST_TIME)
			{
				timer->lastTime.tv_sec = delta.tv_sec;
				timer->lastTime.tv_nsec = delta.tv_nsec;

				setText (timer, &timer->lastTime);

			}
			else if (timer->mode == BEST_TIME)
			{
				if (timer->bestTime.tv_nsec == 0 && timer->bestTime.tv_sec == 0) {
					timer->bestTime.tv_sec = delta.tv_sec;
					timer->bestTime.tv_nsec = delta.tv_nsec;
				}
				else
				{
					if (timespecCompare(&delta, &timer->bestTime, <))
					{
						timer->bestTime.tv_sec = delta.tv_sec;
						timer->bestTime.tv_nsec = delta.tv_nsec;
					}
				}

				setText (timer, &timer->bestTime);
			}

			timer->buttonPressed = true;
			clock_gettime (CLOCK_MONOTONIC, &timer->startTime);
		}
		else
		{
			timer->running = true;
			timer->buttonPressed = true;
			clock_gettime (CLOCK_MONOTONIC, &timer->startTime);
		}
	}

	if ((value < timer->config.threshold) == timer->buttonPressed)
	{
		timer->buttonPressed = false;
	}

	if (timer->running)
	{
		if (timer->mode == CURRENT_TIME)
		{

			clock_gettime (CLOCK_MONOTONIC, &timer->currentTime);
			struct timespec delta = timespecSub (&timer->currentTime, &timer->startTime);

			setText (timer, &delta);
		}
	}
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
			.widget	= gtk_grid_new (),
		},
		.config		= *config,
		.database	= database,
	};

	// Timer Initialization
	timer->currentTime.tv_sec = 0;
	timer->currentTime.tv_nsec = 0;

	timer->startTime.tv_sec = 0;
	timer->startTime.tv_nsec = 0;

	timer->lastTime.tv_nsec = 0;
	timer->lastTime.tv_sec = 0;

	timer->bestTime.tv_nsec = 0;
	timer->bestTime.tv_sec = 0;

	timer->overlay = gtk_overlay_new ();
	timer->timer = gtk_label_new ("--:--:---");

	// TODO(DiBacco): find where deallocation is taking place
	timer->formattedTime = malloc (sizeof (char*) + 1);
	if (timer->formattedTime == NULL)
		return NULL;

	gtkLabelSetColor (GTK_LABEL (timer->timer), &timer->config.borderColor);
	gtk_widget_set_halign(timer->timer, GTK_ALIGN_START);

	timer->area = gtk_drawing_area_new ();

	gtk_overlay_set_child (GTK_OVERLAY (timer->overlay), timer->area);
	gtk_overlay_add_overlay (GTK_OVERLAY (timer->overlay), timer->timer);
	gtk_grid_attach (GTK_GRID (CAN_WIDGET_TO_WIDGET (timer)), timer->overlay, 0, 0, 1, 1);

	gtk_drawing_area_set_content_width (GTK_DRAWING_AREA (timer->area), config->width);
	gtk_drawing_area_set_content_height (GTK_DRAWING_AREA (timer->area), config->height);
	gtk_drawing_area_set_draw_func (GTK_DRAWING_AREA (timer->area), draw, timer, NULL);

	timer->running = false;
	timer->buttonPressed = false;

	timer->signalIndex = canDatabaseFindSignal (database, config->signalName);

    // Update initial value
	update (timer);

	// Cast into the base type
	return (canWidget_t*) timer;
}

canWidget_t* canLabelTimerLoad (canDatabase_t* database, cJSON* config)
{
	uint16_t width;
	if (jsonGetUint16_t (config, "width", &width) != 0)
		return NULL;

	uint16_t height;
	if (jsonGetUint16_t (config, "height", &height) != 0)
		return NULL;

	char* signalName;
	if (jsonGetString(config, "signalName", &signalName) != 0)
		return NULL;

	float threshold;
	if (jsonGetFloat(config, "threshold", &threshold) != 0)
		return NULL;

	char* backgroundColor;
	if (jsonGetString (config, "backgroundColor", &backgroundColor) != 0)
		return NULL;

	float boarderThickness;
	if (jsonGetFloat (config, "boarderThickness", &boarderThickness) != 0)
		return NULL;

	char* borderColor;
	if (jsonGetString (config, "borderColor", &borderColor) != 0)
		return NULL;

	char* fontColor;
	if (jsonGetString (config, "fontColor", &fontColor) != 0)
		return NULL;

	cJSON* cjsonModes;
	if (jsonGetObject (config, "modes", &cjsonModes) != 0)
		return NULL;

	// TODO: dereference modes
	size_t modeCount = (size_t) cJSON_GetArraySize (cjsonModes);
	char** modes = malloc (sizeof (char*) * modeCount);

	for (size_t modeIndex = 0; modeIndex < modeCount; ++modeIndex)
		modes [modeIndex] = cJSON_GetArrayItem (cjsonModes, modeIndex)->valuestring;

	return canLabelTimerInit (database, &(canLabelTimerConfig_t)
	{
		.width  			= width,
		.height 			= height,
		.signalName			= signalName,
		.threshold 			= threshold,
		.backgroundColor 	= gdkHexToColor (backgroundColor),
		.borderColor 		= gdkHexToColor (borderColor),
		.fontColor 			= gdkHexToColor (fontColor),
		.borderThickness 	= boarderThickness,
		.modeCount			= modeCount,
		.modes 				= modes
	});
}

size_t canLabelTimerSetMode (void* widget, char* mode)
{
	canLabelTimer_t* timer = widget;

	if (strcmp (mode, "current") == 0)
		timer->mode = CURRENT_TIME;

	else if (strcmp (mode, "last") == 0)
		timer->mode = LAST_TIME;

	else if (strcmp (mode, "best") == 0)
		timer->mode = BEST_TIME;

	else
	{
		debugPrintf ("Failed to set mode of unknown type: %s\n", mode);
		return -1;
	}

	return 0;
}

void canLabelTimerTrySetFont (void* widget, char* font)
{
	canLabelTimer_t* timer = widget;
	gtkTryLabelSetFont (timer->timer, font);
}

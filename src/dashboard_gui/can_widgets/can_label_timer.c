#include "can_label_timer.h"

typedef struct
{
	canWidgetVmt_t vmt;
	canLabelTimerConfig_t config;
	canDatabase_t* database;
} canLabelTimer_t;

static void update (void* widget)
{
	float value;
	canLabelTimer_t* label = widget;
	canDatabaseGetFloat(label->database, label->config.signalIndex, &value);

	if (value && !label->config.running)
	{
		label->config.running = true;
		clock_gettime (CLOCK_REALTIME, &label->config.startTime);
	}

	clock_gettime (CLOCK_REALTIME, &label->config.currentTime);

	// char* f = malloc(sizeof (char*));
	// sprintf(f, "%d:%d:%d");

	gtk_label_set_text (GTK_LABEL (CAN_WIDGET_TO_WIDGET (label)), "00:00:00");
}

canWidget_t* canLabelTimerInit (canDatabase_t* database, canLabelTimerConfig_t* config)
{
    // Allocate the object
	canLabelTimer_t* label = malloc (sizeof (canLabelTimer_t));
	if (label == NULL)
		return NULL;

    // Init the object
	*label = (canLabelTimer_t)
	{
		.vmt =
		{
			.update = update,
			.widget	= gtk_label_new ("00:00:00")
		},
		.config		= *config,
		.database	= database,
	};

	label->config.running = false;
	label->config.signalIndex = canDatabaseFindSignal (database, "WHEEL_BUTTON_TOP_LEFT");

    // Update initial value
	update (label);

	// Cast into the base type
	return (canWidget_t*) label;
}

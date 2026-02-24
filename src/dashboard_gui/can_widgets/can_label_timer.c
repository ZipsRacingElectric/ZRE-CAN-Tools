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

	// TODO(DiBacco): change condition to value > 0.
	if (value && !label->config.running)
	{
		label->config.running = true;
		clock_gettime (CLOCK_REALTIME, &label->config.startTime);
	}

	clock_gettime (CLOCK_REALTIME, &label->config.currentTime);
	struct timespec delta = timespecSub (&label->config.currentTime, &label->config.startTime);

	char* time = malloc(sizeof (char));
	sprintf(time, "%02ld:%02ld:%03ld",
		delta.tv_sec / 60,
		delta.tv_sec % 60,
		delta.tv_nsec / 1000000
	);

	gtk_label_set_text (GTK_LABEL (CAN_WIDGET_TO_WIDGET (label)), time);
	free (time);
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

// Header
#include "can_label_bool.h"

// Includes
#include "cjson/cjson_util.h"

typedef enum
{
	CAN_LABEL_BOOL_INVALID,
	CAN_LABEL_BOOL_ACTIVE,
	CAN_LABEL_BOOL_INACTIVE
} canLabelBoolState_t;

typedef struct
{
	// Every CAN widget must start with the VMT.
	canWidgetVmt_t vmt;

	// Configuration
	canLabelBoolConfig_t config;

	// CAN database reference.
	canDatabase_t* database;

	// CAN database signal index.
	ssize_t index;

	// Last state.
	canLabelBoolState_t state;
} canLabelBool_t;

static void update (void* widget)
{
	canLabelBool_t* label = widget;

	// Determine the current state based on the signal value
	canLabelBoolState_t state;
	float value;
	if (canDatabaseGetFloat (label->database, label->index, &value) == CAN_DATABASE_VALID)
	{
		if ((value >= label->config.threshold) != label->config.inverted)
			state = CAN_LABEL_BOOL_ACTIVE;
		else
			state = CAN_LABEL_BOOL_INACTIVE;
	}
	else
		state = CAN_LABEL_BOOL_INVALID;

	// Only update if the state has changed
	if (state != label->state)
	{
		label->state = state;

		const char* str;
		switch (state)
		{
		case CAN_LABEL_BOOL_ACTIVE:
			str = label->config.activeValue;
			break;

		case CAN_LABEL_BOOL_INACTIVE:
			str = label->config.inactiveValue;
			break;

		case CAN_LABEL_BOOL_INVALID:
			str = label->config.invalidValue;
			break;
		}

		gtk_label_set_text (GTK_LABEL (label->vmt.widget), str);
	}
}

canWidget_t* canLabelBoolInit (canDatabase_t* database, canLabelBoolConfig_t* config)
{
	// Allocate the object
	canLabelBool_t* label = malloc (sizeof (canLabelBool_t));
	if (label == NULL)
		return NULL;

	// Init the object
	*label = (canLabelBool_t)
	{
		.vmt =
		{
			.update	= update,
			.widget	= gtk_label_new (config->invalidValue)
		},
		.config		= *config,
		.database	= database,
		.index		= canDatabaseFindSignal (database, config->signalName),
		.state		= CAN_LABEL_BOOL_INVALID
	};

	// Validate the signal exists
	if (label->index < 0)
		return NULL;

	// Update initial value
	update (label);

	// Cast into the base type
	return (canWidget_t*) label;
}

canWidget_t* canLabelBoolLoad (canDatabase_t* database, cJSON* config, canLabelBoolStyle_t* parentStyle)
{
	canLabelBoolConfig_t widgetConfig;

	// Load config fields. Exit early is required field is not specified.

	if (jsonGetString (config, "signalName", &widgetConfig.signalName) != 0)
		return NULL;

	if (jsonGetString (config, "activeValue", &widgetConfig.activeValue) != 0)
		return NULL;

	if (jsonGetString (config, "inactiveValue", &widgetConfig.inactiveValue) != 0)
		return NULL;

	if (jsonGetString (config, "invalidValue", &widgetConfig.invalidValue) != 0)
		return NULL;

	if (jsonGetFloat (config, "threshold", &widgetConfig.threshold) != 0)
		return NULL;

	jsonGetBool (config, "inverted", &widgetConfig.inverted);

	// No style to load right now.
	(void) parentStyle;

	return canLabelBoolInit (database, &widgetConfig);
}

void canLabelBoolLoadStyle (cJSON* config, canLabelBoolStyle_t* style, canLabelBoolStyle_t* parent)
{
	(void) config;
	(void) style;
	(void) parent;

	// Nothing to load right now.

	return;
}
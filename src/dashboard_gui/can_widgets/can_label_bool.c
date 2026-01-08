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
	canWidgetVmt_t vmt;
	canLabelBoolConfig_t config;
	canLabelBoolState_t state;
	canDatabase_t* database;
	ssize_t index;
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
		.state		= CAN_LABEL_BOOL_INVALID,
		.database	= database,
		.index		= canDatabaseFindSignal (database, config->signalName)
	};

	// Update initial value
	update (label);

	// Cast into the base type
	return (canWidget_t*) label;
}

canWidget_t* canLabelBoolLoad (canDatabase_t* database, cJSON* config)
{
	char* widgetType;
	if (jsonGetString (config, "type", &widgetType) != 0)
		return NULL;

	if (strcmp (widgetType, "canLabelBool_t") != 0)
		return NULL;

	char* signalName;
	if (jsonGetString (config, "signalName", &signalName) != 0)
		return NULL;

	char* activeValue;
	if (jsonGetString (config, "activeValue", &activeValue) != 0)
		return NULL;

	char* inactiveValue;
	if (jsonGetString (config, "inactiveValue", &inactiveValue) != 0)
		return NULL;

	char* invalidValue;
	if (jsonGetString (config, "invalidValue", &invalidValue) != 0)
		return NULL;

	float threshold;
	if (jsonGetFloat (config, "threshold", &threshold) != 0)
		return NULL;

	bool inverted;
	if (jsonGetBool (config, "inverted", &inverted) != 0)
		return NULL;

	return canLabelBoolInit (database, &(canLabelBoolConfig_t)
	{
		.signalName		= signalName,
		.activeValue	= activeValue,
		.inactiveValue	= inactiveValue,
		.invalidValue	= invalidValue,
		.threshold		= threshold,
		.inverted		= inverted
	});
}
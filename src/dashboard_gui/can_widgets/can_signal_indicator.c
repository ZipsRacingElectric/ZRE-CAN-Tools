// Header
#include "can_signal_indicator.h"

// Includes
#include "../gtk_util.h"
#include "cjson/cjson_util.h"

typedef enum {
    CAN_SIGNAL_INDICATOR_NO_FAULT,
    CAN_SIGNAL_INDICATOR_FAULT,
    CAN_SIGNAL_INDICATOR_INVALID
} canSignalIndicatorState_t;

typedef struct
{
    canWidgetVmt_t vmt;
	canSignalIndicatorConfig_t config;
	canDatabase_t* database;
	ssize_t index;
    canSignalIndicatorState_t state;

    GtkWidget* signalNameLabel;
    GtkWidget* signalValueLabel;

} canSignalIndicator_t;

static void update (void* widget)
{
    canSignalIndicator_t* indicator = widget;

    // Get the state of the indicator
    canSignalIndicatorState_t state;
    float value;
    if (canDatabaseGetFloat (indicator->database, indicator->index, &value) == CAN_DATABASE_VALID) {
        if (indicator->config.inverted == value)
            state = CAN_SIGNAL_INDICATOR_FAULT;
        else
            state = CAN_SIGNAL_INDICATOR_NO_FAULT;
    }
    else
        state = CAN_SIGNAL_INDICATOR_INVALID;

    // If the state has changed, then trigger a redraw
    if (state != indicator->state)
    {
        indicator->state = state;
        gtk_label_set_text (GTK_LABEL(indicator->signalValueLabel), "New Value");
    }
}

canWidget_t* canSignalIndicatorInit (canDatabase_t* database, canSignalIndicatorConfig_t* config)
{
    float signalValue = 0;
    size_t signalIndex = canDatabaseFindSignal (database, config->signalName);

    if ((int) signalIndex == -1) {
        printf ("Invalid Signal Name.\n");
    }

    // Allocate memory for the object
    canSignalIndicator_t* indicator = malloc (sizeof (canSignalIndicator_t));
    if (indicator == NULL)
        return NULL;

    // Initialize the object
    *indicator = (canSignalIndicator_t)
    {
        .vmt =
        {
            .update = update,
            .widget = gtk_grid_new ()
        },
        .config     = *config,
        .database   = database,
        .index      = signalIndex,
        .state      = CAN_SIGNAL_INDICATOR_INVALID
    };

    // TODO: properly set signal state
    canDatabaseSignalState_t signalState = canDatabaseGetFloat (database, signalIndex, &signalValue);

    gtk_grid_set_row_spacing(GTK_GRID (indicator->vmt.widget), 10);
	gtk_grid_set_column_spacing(GTK_GRID (indicator->vmt.widget), 10);

    indicator->signalNameLabel = gtk_label_new (config->signalName);
    gtk_grid_attach (GTK_GRID(indicator->vmt.widget), indicator->signalNameLabel, 0, 0, 1, 1);

    char* str = malloc (sizeof (str));
    snprintf(str, sizeof(str), "%.2f", signalValue);

    indicator->signalValueLabel = gtk_label_new (str);
    gtk_grid_attach (GTK_GRID(indicator->vmt.widget), indicator->signalValueLabel, 1, 0, 1, 1);

	// Update the initial value
	update (indicator);

    free (str);

	// Cast into the base type
	return (canWidget_t*) indicator;
}

bool canSignalIndicatorGetState (void* widget)
{
    canSignalIndicator_t* indicator = widget;
    return indicator->state;
}

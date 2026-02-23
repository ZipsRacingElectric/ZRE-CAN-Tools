// Header
#include "can_signal_indicator.h"

// Includes
#include "../gtk_util.h"
#include "cjson/cjson_util.h"

// TODO(DiBacco): implement state of the can signal indicator (if necessary).
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
    canSignalIndicatorState_t state;

    GtkWidget* signalNameLabel;
    GtkWidget* signalValueLabel;

} canSignalIndicator_t;

static void update (void* widget)
{
    canSignalIndicator_t* indicator = widget;
    canDatabase_t* database = indicator->database;
    char** signalNames = indicator->config.signalNames;
    size_t signalNamesSize = indicator->config.signalNamesSize;

    for (size_t signalIndex = 0; signalIndex < signalNamesSize; ++signalIndex) {

        float signalValue;
        char* signalName = signalNames[signalIndex];
        size_t globalIndex = canDatabaseFindSignal (database, signalName);

        if (canDatabaseGetFloat (database, globalIndex, &signalValue) != 0)
        {
            // TODO(DiBacco): set the state of the indicator
            char* strValue = malloc (sizeof (char*));
            if (! strValue) return;

            snprintf (strValue, sizeof (strValue), "%.2f", signalValue);

            gtk_label_set_text (GTK_LABEL(indicator->signalNameLabel), signalName);
            gtk_label_set_text (GTK_LABEL(indicator->signalValueLabel), strValue);

            free (strValue);

            return;
        }
    }

    gtk_label_set_text (GTK_LABEL(indicator->signalNameLabel), "No Fault");
    gtk_label_set_text (GTK_LABEL(indicator->signalValueLabel), "-------");
}

canWidget_t* canSignalIndicatorInit (canDatabase_t* database, canSignalIndicatorConfig_t* config)
{
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
        .state      = CAN_SIGNAL_INDICATOR_INVALID
    };

    gtk_grid_set_row_spacing (GTK_GRID (indicator->vmt.widget), 10);
	gtk_grid_set_column_spacing (GTK_GRID (indicator->vmt.widget), 10);

    for (size_t signalIndex = 0; signalIndex < config->signalNamesSize; ++signalIndex)
    {
        float signalValue = 0;
        char* signalName = config->signalNames[signalIndex];
        size_t globalIndex = canDatabaseFindSignal (database, signalName);

        if (canDatabaseGetFloat (database, globalIndex, &signalValue) != 0)
        {
            // TODO(DiBacco): set the state of the indicator
            indicator->signalNameLabel = gtk_label_new (signalName);
            gtk_grid_attach (GTK_GRID(indicator->vmt.widget), indicator->signalNameLabel, 0, 0, 1, 1);

            char* strValue = malloc (sizeof (char*));
            if (! strValue) return NULL;

            snprintf (strValue, sizeof (strValue), "%.2f", signalValue);

            indicator->signalValueLabel = gtk_label_new (strValue);
            gtk_grid_attach (GTK_GRID(indicator->vmt.widget), indicator->signalValueLabel, 1, 0, 1, 1);

	        // Update the initial value
	        update (indicator);

            free (strValue);

	        // Cast into the base type
	        return (canWidget_t*) indicator;
        }
    }

    indicator->signalNameLabel = gtk_label_new ("No Fault");
    gtk_grid_attach (GTK_GRID(indicator->vmt.widget), indicator->signalNameLabel, 0, 0, 1, 1);

    indicator->signalValueLabel = gtk_label_new ("-------");
    gtk_grid_attach (GTK_GRID(indicator->vmt.widget), indicator->signalValueLabel, 1, 0, 1, 1);

    return NULL;
}
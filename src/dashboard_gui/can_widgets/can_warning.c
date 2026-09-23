// Header
#include "can_warning.h"

// Includes
#include "cjson/cjson_util.h"
#include "../gtk_util.h"

typedef struct 
{
// Every CAN widget must start with the VMT
	canWidgetVmt_t vmt;

	// Configuration
	canWarningConfig_t config;

	// CAN database reference
	canDatabase_t* database;

    // CAN database signal index
    ssize_t index;

    float blinkTimer;

    GtkWidget* textLabel;
    GtkWidget* descriptorOneLabel;
    GtkWidget* descriptorTwoLabel;
} canWarning_t;

static void update (void* widget)
{
    canWarning_t* warning = widget;

    float value;

    if (canDatabaseGetFloat (warning->database, warning->index, &value) != CAN_DATABASE_VALID)
    {
        warning->blinkTimer = 0.0f;
        gtk_widget_set_visible (warning->vmt.widget, false);
        return;
    }

    // Show different warning Limp Mode message based on CAN message bits
    switch ((uint8_t) value)
    {
        case 0b00:
            warning->blinkTimer = 0.0f;
            gtk_widget_set_visible (warning->vmt.widget, false);
            break;

        case 0b01:
            gtk_widget_set_visible (warning->vmt.widget, true);

            if (warning->descriptorOneLabel != NULL)
                gtk_widget_set_visible (warning->descriptorOneLabel, false);

            if (warning->descriptorTwoLabel != NULL)
                gtk_widget_set_visible (warning->descriptorTwoLabel, true);

            break;

        case 0b10:
            gtk_widget_set_visible (warning->vmt.widget, true);

            if (warning->descriptorOneLabel != NULL)
                gtk_widget_set_visible (warning->descriptorOneLabel, true);

            if (warning->descriptorTwoLabel != NULL)
                gtk_widget_set_visible (warning->descriptorTwoLabel, false);

            break;

        case 0b11:
            gtk_widget_set_visible (warning->vmt.widget, true);

            if (warning->descriptorOneLabel != NULL)
                gtk_widget_set_visible (warning->descriptorOneLabel, true);

            if (warning->descriptorTwoLabel != NULL)
                gtk_widget_set_visible (warning->descriptorTwoLabel, true);

            break;

        default:
            warning->blinkTimer = 0.0f;
            gtk_widget_set_visible (warning->vmt.widget, false);
            break;
    }

    // Don't blink if the warning isn't active
    if ((uint8_t) value == 0b00)
        return;

    // Blink warning text
    warning->blinkTimer += 1.0f / 30.0f;

    if (warning->config.textStyle.blinkInterval > 0.0f)
    {
        if (warning->blinkTimer >= warning->config.textStyle.blinkInterval)
            warning->blinkTimer = 0.0f;

        if (warning->blinkTimer < warning->config.textStyle.blinkInterval / 2.0f)
            gtkLabelSetColor (GTK_LABEL (warning->textLabel), &warning->config.textStyle.fontColor);
        else
            gtkLabelSetColor (GTK_LABEL (warning->textLabel), &warning->config.textStyle.blinkColor);
    }
    else
    {
        gtkLabelSetColor (GTK_LABEL (warning->textLabel), &warning->config.textStyle.fontColor);
    }
}

canWidget_t* canWarningInit (canDatabase_t* database, canWarningConfig_t* config)
{
    // Allocate the object
    canWarning_t* warning = malloc (sizeof (canWarning_t));
    if (warning == NULL)
        return NULL;

    // Create the fixed container that holds all warning labels
    GtkWidget* fixed = gtk_fixed_new ();

    // Create warning text
    GtkWidget* textLabel = gtk_label_new (config->text);

    gtkLabelSetColor (GTK_LABEL (textLabel), &config->textStyle.fontColor);

    if (config->textStyle.font != NULL)
        gtkLabelSetFont (GTK_LABEL (textLabel), config->textStyle.font);

    // Create descriptor 1
    GtkWidget* descriptorOneLabel = NULL;

      if (config->descriptorOne != NULL)
    {
        descriptorOneLabel = gtk_label_new (config->descriptorOne);

        gtkLabelSetColor (GTK_LABEL (descriptorOneLabel), &config->descriptorOneStyle.fontColor);

        if (config->descriptorOneStyle.font != NULL)
            gtkLabelSetFont (GTK_LABEL (descriptorOneLabel), config->descriptorOneStyle.font);

        gtk_fixed_put (GTK_FIXED (fixed), descriptorOneLabel, config->descriptorOneXPosition, config->descriptorOneYPosition);
    }

    // Create descriptor 2
    GtkWidget* descriptorTwoLabel = NULL;

      if (config->descriptorTwo != NULL)
    {
        descriptorTwoLabel = gtk_label_new (config->descriptorTwo);

        gtkLabelSetColor (GTK_LABEL (descriptorTwoLabel), &config->descriptorTwoStyle.fontColor);

        if (config->descriptorTwoStyle.font != NULL)
            gtkLabelSetFont (GTK_LABEL (descriptorTwoLabel), config->descriptorTwoStyle.font);

        gtk_fixed_put (GTK_FIXED (fixed), descriptorTwoLabel, config->descriptorTwoXPosition, config->descriptorTwoYPosition);
    }

    // Place the labels inside the warning container
    gtk_fixed_put (GTK_FIXED (fixed), textLabel, config->textXPosition, config->textYPosition);

    // Initialize the object.
    *warning = (canWarning_t)
    {
        .vmt =
        {
            .update = update,

            // The entire warning is now the fixed container
            .widget = fixed
        },

        .config = *config,
        .database = database,
        .index = canDatabaseFindSignal(database, config->signalName),

        .textLabel = textLabel,
        .descriptorOneLabel = descriptorOneLabel,
        .descriptorTwoLabel = descriptorTwoLabel
    };

    // Validate the signal exists
	if (warning->index < 0)
		return NULL;

    // Set initial visibility
    gtk_widget_set_visible (warning->vmt.widget, true);

    // Update the initial value
    update (warning);

    return (canWidget_t*) warning;
}


canWidget_t* canWarningLoad (canDatabase_t* database, cJSON* config, canWarningStyle_t* parentStyle)
{
    canWarningConfig_t widgetConfig = {0};

    // Required CAN signal configuration.
    if (jsonGetString (config, "signalName", &widgetConfig.signalName) != 0)
        return NULL;

    if (jsonGetFloat (config, "threshold", &widgetConfig.threshold) != 0)
        return NULL;

    // Warning text.
    if (jsonGetString (config, "text", &widgetConfig.text) != 0)
        return NULL;
    if (jsonGetUint16_t (config, "textXPosition", &widgetConfig.textXPosition) != 0)
        return NULL;
    if (jsonGetUint16_t (config, "textYPosition", &widgetConfig.textYPosition) != 0)
        return NULL;

    // Descriptor 1.
    if (jsonGetString (config, "descriptorOne", &widgetConfig.descriptorOne) == 0)

    if (jsonGetUint16_t (config, "descriptorOneXPosition", &widgetConfig.descriptorOneXPosition) != 0)
        return NULL;
    if (jsonGetUint16_t (config, "descriptorOneYPosition", &widgetConfig.descriptorOneYPosition) != 0)
        return NULL;

    // Descriptor 2.
    if (jsonGetString (config, "descriptorTwo", &widgetConfig.descriptorTwo) == 0)

    if (jsonGetUint16_t (config, "descriptorTwoXPosition", &widgetConfig.descriptorTwoXPosition) != 0)
        return NULL;
    if (jsonGetUint16_t (config, "descriptorTwoYPosition", &widgetConfig.descriptorTwoYPosition) != 0)
        return NULL;

    // Load individual styles.
    cJSON* textStyleConfig = jsonGetObjectV2 (config, "textStyle");
    canWarningLoadStyle (textStyleConfig, &widgetConfig.textStyle, parentStyle);

    cJSON* descriptorOneStyleConfig = jsonGetObjectV2 (config, "descriptorOneStyle");
    canWarningLoadStyle (descriptorOneStyleConfig, &widgetConfig.descriptorOneStyle, parentStyle);

    cJSON* descriptorTwoStyleConfig = jsonGetObjectV2 (config, "descriptorTwoStyle");
    canWarningLoadStyle (descriptorTwoStyleConfig, &widgetConfig.descriptorTwoStyle, parentStyle);

    return canWarningInit (database, &widgetConfig);
}


void canWarningLoadStyle (cJSON* config, canWarningStyle_t* style, canWarningStyle_t* parent)
{
    if (parent != NULL)
    {
        // If a parent is specified, inherit everything.
        *style = *parent;
    }
    else
    {
        // If no parent is specified, use default values.
        *style = (canWarningStyle_t)
        {
            .font = "Sans Bold 32",
            .fontColor = gdkHexToColor ("#F7D616"),
            .blinkColor = gdkHexToColor ("#F0AA29"),
            .blinkInterval = 0.0f
        };
    }

    // If no config was provided, use what we have.
    if (config == NULL)
        return;

    // Load style fields, if specified.
    jsonGetString (config, "font", &style->font);

    char* color;

    if (jsonGetString (config, "fontColor", &color) == 0)
        style->fontColor = gdkHexToColor (color);

    if (jsonGetString (config, "blinkColor", &color) == 0)
        style->blinkColor = gdkHexToColor (color);

    jsonGetFloat (config, "blinkInterval", &style->blinkInterval);
}
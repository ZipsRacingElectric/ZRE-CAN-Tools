#ifndef CAN_WARNING
#define CAN_WARNING

// CAN Warinig Objects ---------------------------------------------------------------------------------------------
// 
// Author: Jake Nowak
// Date Created: 2026.09.10
//
// Description: File that creates warning objects that get overlayed on top of the warning screen.

// Includes --------------------------------------------------------------------------------------------------------

// Includes
#include "can_widget_internal.h"
#include "can_database/can_database.h"
#include "cjson/cjson.h"

// GTK
#include <gtk/gtk.h>

// Datatypes ------------------------------------------------------------------------------------------------------------------

typedef struct 
{
    ///@brief The font size of the warning text.
    char* font;

    ///@brief The default color of the warning text.
    GdkRGBA fontColor; 

    ///@brief The color the warning text will blink.
    GdkRGBA blinkColor;

    ///@brief The amount of time, in seconds, of the interval to blink at. Use 0 for no blinking. Default 0.
    float blinkInterval;
} canWarningStyle_t;

typedef struct 
{
    ///@brief CAN signal that activates the warning.
    char* signalName;

    ///@brief Value at which the warning becomes active.
    float threshold;

    ///@brief The text that displays when the warning is active.
    char* text;

    uint16_t textXPosition;
    uint16_t textYPosition;
        
    ///@brief The style of the warning text
    canWarningStyle_t textStyle;

    ///@brief The descriptor for why the warning happended.
    char* descriptorOne;

    uint16_t descriptorOneXPosition;
    uint16_t descriptorOneYPosition;

    ///@brief The style of the warning descriptors
    canWarningStyle_t descriptorOneStyle;

    ///@brief The descriptor for why the warning happended.
    char* descriptorTwo;

    uint16_t descriptorTwoXPosition;
    uint16_t descriptorTwoYPosition;

    ///@brief The style of the warning descriptors
    canWarningStyle_t descriptorTwoStyle;
} canWarningConfig_t;

// Functions ------------------------------------------------------------------------------------------------------------------

/**
 * @brief Creates and initializes a template CAN widget.
 * @param database The CAN database to bind to.
 * @param config The configuration to use. Note a copy is made, so temporary pointers are acceptable.
 * @return The created widget, if successful, @c NULL otherwise.
 */
canWidget_t* canWarningInit (canDatabase_t* database, canWarningConfig_t* config);

/**
 * @brief Creates and initializes a template CAN widget from a JSON configuration.
 * @param database The CAN database to bind to.
 * @param config The JSON configuration to load from.
 * @param parentStyle The parent style to inherit from, or @c NULL .
 * @return The created widget, if successful, @c NULL otherwise.
 */
canWidget_t* canWarningLoad (canDatabase_t* database, cJSON* config, canWarningStyle_t* parentStyle);

/**
 * @brief Loads a template CAN widget's style from a JSON configuration.
 * @param config JSON configuration to use.
 * @param style The style to load into.
 * @param parent The parent style to inherit from, or @c NULL .
 */
void canWarningLoadStyle (cJSON* config, canWarningStyle_t* style, canWarningStyle_t* parent);

#endif // CAN_WARNING
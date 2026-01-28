#ifndef STYLIZED_BUTTON_H
#define STYLIZED_BUTTON_H

// Stylized GTK Button Widget -------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2026.01.03
//
// Description: A more stylized implementation of a button widget in GTK.

// Includes -------------------------------------------------------------------------------------------------------------------

// GTK
#include <gtk/gtk.h>

// Datatypes ------------------------------------------------------------------------------------------------------------------

typedef void (stylizedButtonCallback_t) (GtkWidget* widget, gpointer data);

typedef struct
{
	/// @brief The default / minimum width of the widget.
	int width;

	/// @brief The default / minimum height of the widget.
	int height;

	/// @brief The thickness, in pixels, of the button's border.
	float borderThickness;

	/// @brief The label of the button.
	const char* label;

	/// @brief The color of the button's background.
	GdkRGBA backgroundColor;

	/// @brief The color of the button's border / frame.
	GdkRGBA borderColor;

	/// @brief The background color of the button when selected.
	GdkRGBA selectedColor;

	/// @brief The color of the button's indicator.
	GdkRGBA indicatorColor;
} stylizedButtonConfig_t;

typedef struct
{
	stylizedButtonConfig_t config;
	stylizedButtonCallback_t* callback;
	void* arg;
	bool held;
	GtkWidget* widget;
	GtkDrawingArea* drawingArea;
	GtkLabel* label;
} stylizedButton_t;

/// @brief Converts a @c stylizedButton_t into a GTK widget.
#define STYLIZED_BUTTON_TO_WIDGET(button) ((button)->widget)

/// @brief Converts a @c stylizedButton_t into a GTK label. Note this label is a child widget.
#define STYLIZED_BUTTON_TO_LABEL(button) ((button)->label)

// Functions ------------------------------------------------------------------------------------------------------------------

/**
 * @brief Creates and initializes a button widget.
 * @param callback The function to call when the button is pressed. Use @c NULL to ignore.
 * @param arg The argument to pass to the callback.
 * @param config The configuration to use.
 * @return The created button, if successful, @c NULL otherwise.
 */
stylizedButton_t* stylizedButtonInit (stylizedButtonCallback_t* callback, void* arg, stylizedButtonConfig_t* config);

#endif // STYLIZED_BUTTON_H
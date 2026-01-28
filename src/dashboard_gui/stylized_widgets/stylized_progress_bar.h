#ifndef STYLIZED_PROGRESS_BAR_H
#define STYLIZED_PROGRESS_BAR_H

// Stylized GTK Progress Bar Widget -------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2026.01.04
//
// Description: A more stylized implementation of a progress bar widget in GTK.

// Includes -------------------------------------------------------------------------------------------------------------------

// GTK
#include <gtk/gtk.h>

// Datatypes ------------------------------------------------------------------------------------------------------------------

typedef struct
{
	/// @brief The default / minimum width of the widget.
	int width;

	/// @brief The default / minimum height of the widget.
	int height;

	/// @brief The thickness, in pixels, of the button's border.
	float borderThickness;

	/// @brief The orientation of the progress bar (direction of fill).
	GtkOrientation orientation;

	/// @brief Indicates which direction the bar fills. False => left to right / bottom to top.
	bool inverted;

	/// @brief The color of the bar's unfilled background.
	GdkRGBA backgroundColor;

	/// @brief The color of the bar's filled area.
	GdkRGBA fillColor;

	/// @brief The color of the bar's border / frame.
	GdkRGBA borderColor;
} stylizedProgressBarConfig_t;

typedef struct
{
	GtkWidget* widget;
	stylizedProgressBarConfig_t config;
	float value;
} stylizedProgressBar_t;

#define STYLIZED_PROGRESS_BAR_TO_WIDGET(bar) ((bar)->widget)

// Functions ------------------------------------------------------------------------------------------------------------------

/**
 * @brief Creates and initializes a progress bar widget.
 * @param config The configuration to use.
 * @return The created widget, if successful, @c NULL otherwise.
 */
stylizedProgressBar_t* stylizedProgressBarInit (stylizedProgressBarConfig_t* config);

/**
 * @brief Sets the fill amount of a progress bar.
 * @param bar The bar to set.
 * @param value The fill amount, in range [0, 1].
 */
void stylizedProgressBarSetValue (stylizedProgressBar_t* bar, float value);

#endif // STYLIZED_PROGRESS_BAR_H
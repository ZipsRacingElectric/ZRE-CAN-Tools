#ifndef STYLIZED_FRAME_H
#define STYLIZED_FRAME_H

// Stylized Frame -------------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2026.01.07
//
// Description: A more stylized implementation of a frame in GTK.

// Includes -------------------------------------------------------------------------------------------------------------------

// GTK
#include <gtk/gtk.h>

// Datatypes ------------------------------------------------------------------------------------------------------------------

typedef struct
{
	/// @brief The color of the frame's background.
	GdkRGBA backgroundColor;
	/// @brief The color of the frame's border.
	GdkRGBA borderColor;
	/// @brief The thickness, in pixels, of the frame's border.
	float borderThickness;
	/// @brief The radius, in pixels, of the frame's corners.
	float cornerRadius;
} stylizedFrameConfig_t;

typedef struct
{
	GtkWidget* widget;
	stylizedFrameConfig_t config;
} stylizedFrame_t;

#define STYLIZED_FRAME_TO_WIDGET(frame) ((frame)->widget)

/**
 * @brief Creates and initializes a stylized frame.
 * @param config The configuration to use.
 * @return The created frame, if successful, @c NULL otherwise.
 */
stylizedFrame_t* stylizedFrameInit (stylizedFrameConfig_t* config);

/**
 * @brief Sets the main child of a stylized frame.
 * @param frame The frame to set the child of.
 * @param child The widget to set as the child.
 */
void stylizedFrameSetChild (stylizedFrame_t* frame, GtkWidget* child);

#endif // STYLIZED_FRAME_H
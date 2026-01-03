#ifndef GTK_UTIL
#define GTK_UTIL

// GTK Utilities --------------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2025.12.17
//
// Description: Utilities for working with the GTK library.

// Includes -------------------------------------------------------------------------------------------------------------------

// GTK
#include <gtk/gtk.h>

// Functions ------------------------------------------------------------------------------------------------------------------

/**
 * @brief Sets the font used by a label.
 * @param label The label to set the font of.
 * @param pangoFontDescriptor The font descriptor string to use. See the below URL for more details:
 *   https://docs.gtk.org/Pango/type_func.FontDescription.from_string.html
 */
void gtkLabelSetFont (GtkLabel* label, const char* pangoFontDescriptor);

/**
 * @brief Sets the color of a label.
 * @param label The label to set the color of.
 * @param color The color to set.
 */
void gtkLabelSetColor (GtkLabel* label, const GdkRGBA* color);

/**
 * @brief Converts a hex color string into a GDK color.
 * @param color The hex string of the color, formatted as '#rrggbbaa'. Note alpha is optional.
 * @return The converted color.
 */
GdkRGBA gdkHexToColor (const char* color);

#endif // GTK_UTIL
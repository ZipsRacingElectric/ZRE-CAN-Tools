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

// Macros ---------------------------------------------------------------------------------------------------------------------

/// @brief Forward-compatibility macro. The @c GTK_ALIGN_BASELINE_CENTER enum value replaced the @c GTK_ALIGN_BASELINE value
/// in GTK 4.12 without providing backwards-compatibility, so we do so here.
#if GTK_MINOR_VERSION < 12
#define GTK_ALIGN_BASELINE_CENTER GTK_ALIGN_BASELINE
#endif

// Functions ------------------------------------------------------------------------------------------------------------------

/**
 * @brief Sets the font used by a label.
 * @param label The label to set the font of.
 * @param pangoFontDescriptor The font descriptor string to use. See the below URL for more details:
 *   https://docs.gtk.org/Pango/type_func.FontDescription.from_string.html
 */
void gtkLabelSetFont (GtkLabel* label, const char* pangoFontDescriptor);

/**
 * @brief Attempts to set the font of a label. This function is safe to call even if the widget is not a label, in which case
 * nothing will happen.
 * @param widget The widget to attempt to set the font of.
 * @param pangoFontDescriptor The font descriptor string to use. See the below URL for more details:
 *   https://docs.gtk.org/Pango/type_func.FontDescription.from_string.html
 */
void gtkTryLabelSetFont (GtkWidget* widget, const char* pangoFontDescriptor);

/**
 * @brief Sets the color of a label.
 * @param label The label to set the color of.
 * @param color The color to set.
 */
void gtkLabelSetColor (GtkLabel* label, const GdkRGBA* color);

/**
 * @brief Attempts to set the color of a label. This function is safe to call even if the widget is not a label, in which case
 * nothing will happen.
 * @param widget The widget to attempt to set the color of.
 * @param color The color to set.
 */
void gtkTryLabelSetColor (GtkWidget* widget, const GdkRGBA* color);

/**
 * @brief Attempts to set the X alignment of a label. This function is safe to call even if the widget is not a label, in which
 * case nothing will happen.
 * @param widget The widget to attempt to set the alignment of.
 * @param xalign The alignment to set. See @c gtk_label_set_xalign for more details.
 */
void gtkTryLabelSetXAlign (GtkWidget* widget, float xalign);

/**
 * @brief Attempts to set the Y alignment of a label. This function is safe to call even if the widget is not a label, in which
 * case nothing will happen.
 * @param widget The widget to attempt to set the alignment of.
 * @param yalign The alignment to set. See @c gtk_label_set_yalign for more details.
 */
void gtkTryLabelSetYAlign (GtkWidget* widget, float yalign);

/**
 * @brief Converts a hex color string into a GDK color.
 * @param color The hex string of the color, formatted as '#rrggbbaa'. Note alpha is optional.
 * @return The converted color.
 */
GdkRGBA gdkHexToColor (const char* color);

/**
 * @brief Adds a GDK color stop to a cairo pattern.
 * @param pattern The cairo pattern to add to.
 * @param offset The offset to add the stop at.
 * @param color The GDK color to add.
 */
static inline void gdkCairoPatternAddColorStop (cairo_pattern_t* pattern, double offset, GdkRGBA* color)
{
	cairo_pattern_add_color_stop_rgba (pattern, offset, color->red, color->green, color->blue, color->alpha);
}

#endif // GTK_UTIL
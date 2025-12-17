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

void gtkLabelSetFont (GtkLabel* label, const char* pangoFontDescriptor);

#endif // GTK_UTIL
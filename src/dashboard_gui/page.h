#ifndef PAGE_H
#define PAGE_H

// GUI Page -------------------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2025.12.29
//
// Description: TODO(Barach)

// Includes -------------------------------------------------------------------------------------------------------------------

// GTK
#include <gtk/gtk.h>

// Datatypes ------------------------------------------------------------------------------------------------------------------

typedef void (pageButtonCallback_t) (GtkWidget* widget, gpointer data);

typedef void (pageUpdate_t) (void* page);

typedef void (pageAppendButton_t) (void* page, const char* label, pageButtonCallback_t* callback, void* arg);

/// @brief Virtual method table (and virtual members) for the @c page_t base structure.
typedef struct
{
	pageUpdate_t* update;
	pageAppendButton_t* appendButton;
	GtkWidget* widget;
	GtkWidget* buttonPanel;
	size_t buttonCount;
} pageVmt_t;

typedef struct
{
	pageVmt_t vmt;
} page_t;

// Macros ---------------------------------------------------------------------------------------------------------------------

#define PAGE_TO_WIDGET(page) ((page)->vmt.widget)

// Functions ------------------------------------------------------------------------------------------------------------------

static inline void pageUpdate (page_t* page)
{
	page->vmt.update (page);
}

static inline void pageAppendButton (page_t* page, const char* label, pageButtonCallback_t* callback, void* arg)
{
	page->vmt.appendButton (page, label, callback, arg);
}

#endif // PAGE_H
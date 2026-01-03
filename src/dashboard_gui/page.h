#ifndef PAGE_H
#define PAGE_H

// GUI Page -------------------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2025.12.29
//
// Description: Base polymorphic object for a single page of a GUI. A page is a single screen being rendered at a given time.

// TODO(Barach): Docs

// Includes -------------------------------------------------------------------------------------------------------------------

// GTK
#include <gtk/gtk.h>

// Datatypes ------------------------------------------------------------------------------------------------------------------

typedef void (pageButtonCallback_t) (GtkWidget* widget, gpointer data);

typedef void (pageUpdate_t) (void* page);

typedef void (pageAppendButton_t) (void* page, const char* label, pageButtonCallback_t* callback, void* arg);

/// @brief Virtual method (and member) table for the @c page_t base polymorphic object.
typedef struct
{
	pageUpdate_t* update;
	pageAppendButton_t* appendButton;
	GtkWidget* widget;
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
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

// Includes
#include "can_database/can_database.h"
#include "cjson/cjson.h"

// GTK
#include <gtk/gtk.h>

// Datatypes ------------------------------------------------------------------------------------------------------------------

typedef struct pageStack pageStack_t;

typedef struct
{
	GdkRGBA backgroundColor;
	GdkRGBA fontColor;
	GdkRGBA borderColor;
	float borderThickness;
	GdkRGBA indicatorActiveColor;
	GdkRGBA indicatorInactiveColor;
	int buttonHeight;
	char* buttonFont;
} pageStyle_t;

typedef void (pageButtonCallback_t) (GtkWidget* widget, gpointer data);

typedef void (pageUpdate_t) (void* page);

typedef void (pageAppendButton_t) (void* page, const char* label, pageButtonCallback_t* callback, void* arg, bool currentPage);

/// @brief Virtual method (and member) table for the @c page_t base polymorphic object.
typedef struct
{
	pageUpdate_t* update;
	pageAppendButton_t* appendButton;
	GtkWidget* widget;
	char* name;
	pageStack_t* parent;
} pageVmt_t;

typedef struct
{
	pageVmt_t vmt;
} page_t;

// Macros ---------------------------------------------------------------------------------------------------------------------

#define PAGE_TO_WIDGET(page) ((page)->vmt.widget)

// Functions ------------------------------------------------------------------------------------------------------------------

page_t* pageLoad (cJSON* config, canDatabase_t* database, pageStyle_t* style);

pageStyle_t* pageStyleLoad (cJSON* config, pageStyle_t* parent);

static inline void pageUpdate (page_t* page)
{
	page->vmt.update (page);
}

static inline void pageAppendButton (page_t* page, const char* label, pageButtonCallback_t* callback, void* arg, bool currentPage)
{
	page->vmt.appendButton (page, label, callback, arg, currentPage);
}

static inline char* pageGetName (page_t* page)
{
	return page->vmt.name;
}

static inline void pageSetParent (page_t* page, pageStack_t* parent)
{
	page->vmt.parent = parent;
}

static inline pageStack_t* pageGetParent (page_t* page)
{
	return page->vmt.parent;
}

#endif // PAGE_H
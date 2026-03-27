#ifndef PAGE_H
#define PAGE_H

// GUI Page -------------------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2025.12.29
//
// Description: Base polymorphic object for a single page of a GUI. A page is a single screen being rendered at a given time.

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "../can_widgets/can_widget.h"
#include "can_database/can_database.h"
#include "cjson/cjson.h"

// GTK
#include <gtk/gtk.h>

// Datatypes ------------------------------------------------------------------------------------------------------------------

// Page stack forward declaration
typedef struct pageStack pageStack_t;

typedef struct
{
	/// @brief The style to use for the page's CAN widgets. Default inherits from parent's style, or the widget-specific
	/// defaults if none.
	canWidgetStyle_t widgetStyle;

	/// @brief The color of the page's background, default black.
	GdkRGBA backgroundColor;

	/// @brief The color of the page's labels, default white.
	GdkRGBA fontColor;

	/// @brief The color of the page's borders, default white.
	GdkRGBA borderColor;

	/// @brief The thickness of the page's borders, in pixels. Default 1.5.
	float borderThickness;

	/// @brief The color of the page's button's indicator, when active. Default red.
	GdkRGBA buttonActiveColor;

	/// @brief The color of the page's button's indicator, when inactive. Default dark red.
	GdkRGBA buttonInactiveColor;

	/// @brief The height of the page's buttons, in pixels. Default 80.
	int buttonHeight;

	/// @brief The font of the button's text. Default is GTK's default font.
	char* buttonFont;
} pageStyle_t;

typedef void (pageButtonCallback_t) (GtkWidget* widget, gpointer data);

typedef void (pageUpdate_t) (void* page);

typedef void (pageAppendButton_t) (void* page, const char* label, pageButtonCallback_t* callback, void* arg, bool currentPage, pageStyle_t* style);

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

/// @brief Gets the top-level GTK widget of a page.
#define PAGE_TO_WIDGET(page) ((page)->vmt.widget)

/// @brief Gets the page-specific style of a page.
#define PAGE_STYLE(page) ((page)->style)

/// @brief Gets the base page style of a page.
#define PAGE_BASE_STYLE(page) ((page)->style.baseStyle)

/// @brief Gets the CAN widget style of a page.
#define PAGE_WIDGET_STYLE(page) (&(page)->style.baseStyle->widgetStyle)

// Functions ------------------------------------------------------------------------------------------------------------------

/**
 * @brief Loads a page from a configuration JSON. This function parses the "type" field of the provided JSON to determine which
 * type of page the configuration belongs to, then loads the page via its constructor.
 * @param config The JSON configuration to use. The remainder of the config is parsed as the page-specific config of the page
 * specified by "type".
 * @param databases The array of CAN databases to bind the page's widgets to.
 * @param databaseCount The number of elements in @c databases .
 * @param style The parent base page style to inherit from.
 * @return The created page, if successful, @c NULL otherwise.
 */
page_t* pageLoad (cJSON* config, canDatabase_t* databases, size_t databaseCount, pageStyle_t* style);

/**
 * @brief Loads a base page style from a JSON configuration.
 * @param config The JSON configuration to load from.
 * @param parent The parent base page style to inherit from.
 * @return The loaded base style, if successful, @c NULL otherwise.
 */
pageStyle_t* pageStyleLoad (cJSON* config, pageStyle_t* parent);

/**
 * @brief Updates all the widgets on a page.
 * @param page The page to update.
 */
static inline void pageUpdate (page_t* page)
{
	page->vmt.update (page);
}

/**
 * @brief Adds a new navigation button to a page.
 * @param page The page to append to.
 * @param label The text to label the button with.
 * @param callback The callback to execute when the button is pressed, or @c NULL to ignore.
 * @param arg The argument to pass to the callback.
 * @param currentPage Indicates if the button navigates to this page or not.
 * @param style The base page style to stylize the button with.
 */
static inline void pageAppendButton (page_t* page, const char* label, pageButtonCallback_t* callback, void* arg, bool currentPage, pageStyle_t* style)
{
	page->vmt.appendButton (page, label, callback, arg, currentPage, style);
}

/// @return The name of the page.
static inline char* pageGetName (page_t* page)
{
	return page->vmt.name;
}

/**
 * @brief Sets the parent page stack of a page.
 * @param page The page to use.
 * @param parent The page stack to use.
 */
static inline void pageSetParent (page_t* page, pageStack_t* parent)
{
	page->vmt.parent = parent;
}

/// @return The page's parent page stack.
static inline pageStack_t* pageGetParent (page_t* page)
{
	return page->vmt.parent;
}

#endif // PAGE_H
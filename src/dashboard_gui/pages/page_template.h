#ifndef PAGE_TEMPLATE_H
#define PAGE_TEMPLATE_H

// Template Page --------------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2026.02.09
//
// Description: Template for creating new GUI pages.

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "page.h"
#include "can_database/can_database.h"
#include "cjson/cjson.h"

// GTK
#include <gtk/gtk.h>

// Datatypes ------------------------------------------------------------------------------------------------------------------

typedef struct
{
	pageStyle_t* baseStyle;
} pageTemplateStyle_t;

typedef struct
{
	pageVmt_t vmt;

	pageTemplateStyle_t style;

	GtkGrid* buttonPanel;
	size_t buttonCount;
} pageTemplate_t;

// Functions ------------------------------------------------------------------------------------------------------------------

page_t* pageTemplateLoad (cJSON* config, canDatabase_t* database, pageStyle_t* style);

#endif // PAGE_TEMPLATE_H
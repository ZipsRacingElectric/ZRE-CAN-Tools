#ifndef PAGE_WARNING_H
#define PAGE_WARNING_H

// Status Page ----------------------------------------------------------------------------------------------------------------
//
// Author: Jake Nowak
// Date Created: 2026.9.10
//
// Description: TODO

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "page.h"
#include "../can_widgets/can_warning.h"
#include "cjson/cjson.h"

// GTK
#include <gtk/gtk.h>

// Macros ---------------------------------------------------------------------------------------------------------------------

#define PAGE_WARNING_BUTON_COUNT 7

// Datatypes ------------------------------------------------------------------------------------------------------------------

typedef struct
{
	pageStyle_t* baseStyle;
} pageWarningStyle_t;

typedef struct
{
	pageVmt_t vmt;

	pageWarningStyle_t style;

	canWidget_t** warnings;
	size_t warningCount;

	canDatabase_t** triggerDatabase;
	ssize_t* triggerIndices;

	canDatabase_t* sibDatabase;
	ssize_t sibButtonIndices[PAGE_WARNING_BUTON_COUNT];

	bool buttonPressedLast;
	bool buttonStateInitialized;
} pageWarning_t;

// Functions ------------------------------------------------------------------------------------------------------------------

page_t* pageWarningLoad (cJSON* config, canDatabase_t* databases, size_t databaseCount, pageStyle_t* style);


///@brief Checks if any warning condition on this page is currently active.
///@param pageBase The page to check, must be pageWarning_t
///@return true if at least one warning signal is in a non-zero state.
bool pageWarningIsActive (page_t* pageBase);

void pageWarningResetButtonState(page_t* pageBase);

#endif // PAGE_WARNING_H
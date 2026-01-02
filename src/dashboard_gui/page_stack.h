#ifndef PAGE_STACK_H
#define PAGE_STACK_H

// Page Stack -----------------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2026.01.01
//
// Description: TODO(Barach)

// TODO(Barach): Docs

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "page.h"

// GTK
#include <gtk/gtk.h>

// Datatypes ------------------------------------------------------------------------------------------------------------------

typedef struct
{
	GtkWidget* widget;
	page_t* selectedPage;
} pageStack_t;

typedef struct
{
	pageStack_t* stack;
	page_t* page;
} pageStackPair_t;

#define PAGE_STACK_TO_WIDGET(stack) ((stack)->widget)
#define PAGE_STACK_TO_STACK(stack) GTK_STACK ((stack)->widget)

// Functions ------------------------------------------------------------------------------------------------------------------

pageStack_t* pageStackInit (void);

void pageStackAppend (pageStack_t* stack, page_t* page);

void pageStackSelect (pageStack_t* stack, page_t* page);

void pageStackSelectCallback (GtkWidget* widget, void* arg);

void pageStackUpdate (pageStack_t* stack);

pageStackPair_t* pageStackPairInit (pageStack_t* stack, page_t* page);

#endif // PAGE_STACK_H
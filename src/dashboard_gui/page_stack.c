// Header
#include "page_stack.h"

pageStack_t* pageStackInit (void)
{
	pageStack_t* stack = malloc (sizeof (pageStack_t));
	if (stack == NULL)
		return NULL;

	stack->widget = gtk_stack_new ();
	stack->selectedPage = NULL;

	return stack;
}

void pageStackAppend (pageStack_t* stack, page_t* page)
{
	gtk_stack_add_child (PAGE_STACK_TO_STACK (stack), PAGE_TO_WIDGET (page));
	pageSetParent (page, stack);
	if (stack->selectedPage == NULL)
		pageStackSelect (stack, page);
}

void pageStackSelect (pageStack_t* stack, page_t* page)
{
	stack->selectedPage = page;
	gtk_stack_set_visible_child (PAGE_STACK_TO_STACK (stack), PAGE_TO_WIDGET (page));
	pageUpdate (stack->selectedPage);
}

void pageStackSelectCallback (GtkWidget* widget, void* arg)
{
	(void) widget;
	page_t* page = arg;
	pageStack_t* stack = pageGetParent (page);
	if (stack == NULL)
		return;
	pageStackSelect (stack, page);
}

void pageStackUpdate (pageStack_t* stack)
{
	if (stack->selectedPage != NULL)
		pageUpdate (stack->selectedPage);
}
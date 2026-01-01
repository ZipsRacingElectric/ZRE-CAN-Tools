#ifndef LIST_H
#define LIST_H

// Generic Datatype List ------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2025.10.10
//
// Description: Template object and functions for a list implementation. This list implements an ordered, variable-length
//   collection of variables, most comparable to the vector in C++ or the List in Python. As it must support a variety of
//   different datatypes, this object is implemented via macros, analogous to template objects in C++.
//
// Examples -------------------------------------------------------------------------------------------------------------------
//
// At the top-level scope, the list must be explicitly defined for the specific datatype (or datatypes) it is to be used with.
// This is done via the listDefine(...) macro. Ex:
//
//   listDefine (float)
//
// To instance a list, the list's datatype is given by the list_t(...) macro. Ex:
//
//   list_t (float) exampleInstance;
//
// To initialize a list, the listInit (...) funtion-like macro can be used. Ex:
//
//   listInit (float) (&exampleInstance, 10);
//
// To add, remove, or do any other operations to a list, the appropriate function-like macros can be used. Ex:
//
//   listAppend (float) (&exampleInstance, 1.5f);
//   listTruncate (float) (&exampleInstance);
//
// To convert a list into an array, the list can be destroyed without freeing its dynamically allocated memory using the
// listDestroy macro. Ex:
//
//   size_t arrSize;
//   float* arr = listDestroy (&exampleInstance, &arrSize);
//
// To use a list with a non-trivial datatype, ex pointers, a typedef can be used to trivialize the datatype's name. Ex:
//
//   // Cannot do this:
//   listDefine (float*);
//
//   // Rather we must do:
//   typedef float* floatPtr;
//   listDefine (floatPtr);

// Includes -------------------------------------------------------------------------------------------------------------------

// C Standard Library
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>

// Datatypes ------------------------------------------------------------------------------------------------------------------

/// @brief Datatype defining a list for a specific datatype.
#define list_t(datatype)																									\
	list_ ## datatype

// Function-like Macros -------------------------------------------------------------------------------------------------------

/**
 * @brief Initializes a list.
 * @param list The list to initialize.
 * @param capacity The initial capacity of this list. This does not indicate the size of the list, just the size of the
 * internal array.
 * @return 0 if successful, the error code otherwise.
 */
#define listInit(datatype)																									\
	listInit_ ## datatype

/**
 * @brief Reallocates the internal array of a list.
 * @param list The list to reallocate.
 * @param capacity The new capacity of this list. This does not indicate the size of the list, just the size of the internal
 * array. The size of the list itself remains unchanged.
 * @return 0 if successful, the error code otherwise.
 */
#define listRealloc(datatype)																								\
	listRealloc_ ## datatype

/**
 * @brief De-allocates the internal array of a list.
 * @param list The list to de-allocate.
 */
#define listDealloc(datatype)																								\
	listDealloc_ ## datatype

/**
 * @brief Destroys a list object while preserving the internal array. Used to get an array containing the contents of the list.
 * @param list The list to destroy. Cannot be used after this function.
 * @param size Buffer to write the size of the returned array into.
 * @return An array containing the contents of the list, if successful, @c NULL otherwise. Note @c errno is set on failure.
 */
#define listDestroy(datatype)																								\
	listDestroy_ ## datatype

/**
 * @brief Appends an uninitialized element to the end of a list, re-allocating the list if necessary.
 * @param list The list to append to.
 * @return A pointer to the appended object, if successful, @c NULL otherwise. Note @c errno is set on failure.
 */
#define listAppendUninit(datatype)																							\
	listAppendUninit_ ## datatype

/**
 * @brief Appends an element to the end of a list, re-allocating the list if necessary.
 * @param list The list to append to.
 * @param element The element to append.
 * @return 0 if successful, the error code otherwise.
 */
#define listAppend(datatype)																								\
	listAppend_ ## datatype

/**
 * @brief Truncates the element off of a list.
 * @param list The list to truncate to.
 */
#define listTruncate(datatype)																								\
	listTruncate_ ## datatype

/**
 * @brief Gets the value of an element in a list.
 * @param list The list to get from.
 * @param index The index of the element to get.
 * @return The element at said index.
 */
#define listGet(datatype)																									\
	listGet_ ## datatype

/**
 * @brief Sets the value of an element in a list.
 * @param list The list to set in.
 * @param index The index of the element to set.
 * @param element The value to set.
 */
#define listSet(datatype)																									\
	listSet_ ## datatype

/**
 * @brief Gets a reference to an element in a list.
 * @param list The list to get from.
 * @param index The index of the element to get.
 * @return A pointer to the element at said index.
 */
#define listGetReference(datatype)																							\
	listGetReference_ ## datatype

/// @return An array containing the contents of the list. Note this array is invalid after any operation is performed to the
/// list. The size of this array is indicated by the @c listSize function-like macro.
#define listArray(datatype)																									\
	listArray_ ## datatype

/// @return The size of the list.
#define listSize(datatype)																									\
	listSize_ ## datatype

/**
 * @brief Checks whether a specified element is contained within a list.
 * @param list The list to search within.
 * @param element The element to search for.
 * @return True if the element was found, false otherwise.
 */
#define listContains(datatype)																								\
	listContains_ ## datatype

// Macros ---------------------------------------------------------------------------------------------------------------------

/// @brief Macro for defining a datatype-specific implementation of a list. Can only be used at the top-level scope. Must be
/// used before any function-like macros are used.
#define listDefine(datatype)																								\
	typedef struct																											\
	{																														\
		size_t size;																										\
		size_t capacity;																									\
		datatype* array;																									\
	} list_t (datatype);																									\
																															\
	static inline int listInit (datatype) (list_t (datatype)* list, size_t capacity)										\
	{																														\
		/* Allocate the internal array */																					\
		list->array = malloc (sizeof (datatype) * capacity);																\
		if (list->array == NULL)																							\
		{																													\
			list->size = 0;																									\
			list->capacity = 0;																								\
			return errno;																									\
		}																													\
																															\
		list->size = 0;																										\
		list->capacity = capacity;																							\
		return 0;																											\
	}																														\
																															\
	static inline int listRealloc (datatype) (list_t (datatype)* list, size_t capacity)										\
	{																														\
		/* Reallocate the internal array. */																				\
		list->array = realloc (list->array, sizeof (datatype) * capacity);													\
		if (list->array == NULL)																							\
			return errno;																									\
																															\
		/* Success */																										\
		list->capacity = capacity;																							\
		return 0;																											\
	}																														\
																															\
	static inline void listDealloc (datatype) (list_t (datatype)* list)														\
	{																														\
		/* Deallocate the internal array. */																				\
		free (list->array);																									\
	}																														\
																															\
	static inline datatype* listDestroy (datatype) (list_t (datatype)* list, size_t* size)									\
	{																														\
		/* Shrink the list to the minimum size */																			\
		if (listRealloc (datatype) (list, list->size) != 0)																			\
			return NULL;																									\
																															\
		/* Return the array and its size */																					\
		*size = list->size;																									\
		return list->array;																									\
	}																														\
																															\
	static inline datatype* listAppendUninit (datatype) (list_t (datatype)* list)											\
	{																														\
		/* Check if there is space in the list. */																			\
		if (list->size + 1 > list->capacity)																				\
		{																													\
			/* If we are out of space, double the size of the internal array. */											\
			if (listRealloc (datatype) (list, list->capacity * 2) != 0)														\
				return NULL;																								\
		}																													\
																															\
		/* Add the element to the end of the list */																		\
		++list->size;																										\
																															\
		/* Success, return a reference to the appended object */															\
		return &list->array [list->size - 1];																				\
	}																														\
																															\
	static inline int listAppend (datatype) (list_t (datatype)* list, datatype element)										\
	{																														\
		/* Uninitialized append */																							\
		datatype* slot = listAppendUninit (datatype) (list);																\
		if (slot == NULL)																									\
			return errno;																									\
																															\
		/* Initialize the new element */																					\
		*slot = element;																									\
		return 0;																											\
	}																														\
																															\
	static inline void listTruncate (datatype) (list_t (datatype)* list)													\
	{																														\
		/* Remove the last element, if the list is not empty */																\
		if (list->size > 0)																									\
			--list->size;																									\
	}																														\
																															\
	static inline datatype listGet (datatype) (list_t (datatype)* list, size_t index)										\
	{																														\
		return list->array [index];																							\
	}																														\
																															\
	static inline void listSet (datatype) (list_t (datatype)* list, size_t index, datatype element)							\
	{																														\
		list->array [index] = element;																						\
	}																														\
																															\
	static inline datatype* listGetReference (datatype) (list_t (datatype)* list, size_t index)								\
	{																														\
		return &list->array [index];																						\
	}																														\
																															\
	static inline datatype* listArray (datatype) (list_t (datatype)* list)													\
	{																														\
		return list->array;																									\
	}																														\
																															\
	static inline size_t listSize (datatype) (list_t (datatype)* list)														\
	{																														\
		return list->size;																									\
	}

#endif // LIST_H
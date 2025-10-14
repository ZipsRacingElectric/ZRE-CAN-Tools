#ifndef ARRAY_H
#define ARRAY_H

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
//   arrayDefine (float)
//
// To do any operations on an array, the appropriate function-like macros can be used. Ex:
//
//   arrayContains (float) (exampleArray, 1.5f, sizeof (exampleArray));

// Includes -------------------------------------------------------------------------------------------------------------------

// C Standard Library
#include <stdbool.h>
#include <stddef.h>

// Function-like Macros -------------------------------------------------------------------------------------------------------

/**
 * @brief Checks whether a specified element is contained within an array.
 * @param array The array to search within.
 * @param element The element to search for.
 * @param size The size of the array.
 * @return True if the element was found, false otherwise.
 */
#define arrayContains(datatype)																								\
	arrayContains_ ## datatype

// Macros ---------------------------------------------------------------------------------------------------------------------

/// @brief Macro for defining a datatype-specific implementation of the helper funcitions. Can only be used at the top-level
/// scope. Must be used before any function-like macros are used.
#define arrayDefine(datatype)																								\
																															\
	static inline bool arrayContains (datatype) (datatype* array, datatype element, size_t size)						\
	{																														\
		for (size_t index = 0; index < size; ++index)																		\
			if (array [index] == element)																					\
				return true;																								\
																															\
		return false;																										\
	}

#endif // ARRAY_H
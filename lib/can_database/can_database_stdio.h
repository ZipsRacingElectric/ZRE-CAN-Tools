#ifndef CAN_DATABASE_STDIO_H
#define CAN_DATABASE_STDIO_H

// CAN Database Standard Input / Output ---------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2025.11.18
//
// Description: Helper functions for using standard input & output with the can_database library.

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "can_database.h"

// C Standard Library
#include <stdio.h>

// Functions ------------------------------------------------------------------------------------------------------------------

/**
 * @brief Prints the help page for the DBC File parameter.
 * @param stream The I/O stream to print to.
 * @param indent The indentation to put before each line. Note that spaces are preferred to tabs.
 * @return The number of bytes written if successful, a negative value otherwise.
 */
int fprintCanDbcFileHelp (FILE* stream, char* indent);

/**
 * @brief Prints the value of a float from a CAN database (without needing a reference to the database).
 * @param stream The I/O stream to print to.
 * @param formatValue The format string to use for the value (if valid). Must include the %f format specifier, which will be
 * replaced by the value. Can optionally follow the %f specifier with a %s specifier for the value's unit. No other format
 * specifiers are allowed in this string.
 * @param formatInvalid The format string to use for the value if invalid. Should include the %s format specifier, which will
 * be replaced by '--'. Can optionally follow said specifier with a %s specifier for the value's unit, if known. No other
 * format specifiers are allowed in this string.
 * @param value The value of the unit, if known.
 * @param state The state of the signal.
 * @param unit The unit of the signal, if known.
 * @return The number of bytes written, if successful, a negative value otherwise.
 */
int fprintCanDatabaseFloatStatic (FILE* stream, const char* formatValue, const char* formatInvalid, float value,
	canDatabaseSignalState_t state, const char* unit);

/**
 * @brief Prints the value of a float from a CAN database (without needing a reference to the database).
 * @param str The buffer to print into.
 * @param n The size of the buffer to print into.
 * @param formatValue The format string to use for the value (if valid). Must include the %f format specifier, which will be
 * replaced by the value. Can optionally follow the %f specifier with a %s specifier for the value's unit. No other format
 * specifiers are allowed in this string.
 * @param formatInvalid The format string to use for the value if invalid. Should include the %s format specifier, which will
 * be replaced by '--'. Can optionally follow said specifier with a %s specifier for the value's unit, if known. No other
 * format specifiers are allowed in this string.
 * @param value The value of the unit, if known.
 * @param state The state of the signal.
 * @param unit The unit of the signal, if known.
 * @return The number of bytes written, if successful, a negative value otherwise.
 */
int snprintCanDatabaseFloatStatic (char* str, size_t n, const char* formatValue, const char* formatInvalid, float value,
	canDatabaseSignalState_t state, const char* unit);

/**
 * @brief Prints the value of a float from a CAN database (using a reference to the database).
 * @param stream The I/O stream to print to.
 * @param formatValue The format string to use for the value (if valid). Must include the %f format specifier, which will be
 * replaced by the value. Can optionally follow the %f specifier with a %s specifier for the value's unit. No other format
 * specifiers are allowed in this string.
 * @param formatInvalid The format string to use for the value if invalid. Should include the %s format specifier, which will
 * be replaced by '--'. Can optionally follow said specifier with a %s specifier for the value's unit, if known. No other
 * format specifiers are allowed in this string.
 * @param database The database to pull the value from.
 * @param index The index of the signal to print.
 * @return The number of bytes written, if successful, a negative value otherwise.
 */
int fprintCanDatabaseFloat (FILE* stream, const char* formatValue, const char* formatInvalid,
	canDatabase_t* database, ssize_t index);

/**
 * @brief Prints the value of a float from a CAN database (using a reference to the database).
 * @param str The buffer to print into.
 * @param n The size of the buffer to print into.
 * @param formatValue The format string to use for the value (if valid). Must include the %f format specifier, which will be
 * replaced by the value. Can optionally follow the %f specifier with a %s specifier for the value's unit. No other format
 * specifiers are allowed in this string.
 * @param formatInvalid The format string to use for the value if invalid. Should include the %s format specifier, which will
 * be replaced by '--'. Can optionally follow said specifier with a %s specifier for the value's unit, if known. No other
 * format specifiers are allowed in this string.
 * @param database The database to pull the value from.
 * @param index The index of the signal to print.
 * @return The number of bytes written, if successful, a negative value otherwise.
 */
int snprintCanDatabaseFloat (char* str, size_t n, const char* formatValue, const char* formatInvalid,
	canDatabase_t* database, ssize_t index);

#endif // CAN_DATABASE_STDIO_H
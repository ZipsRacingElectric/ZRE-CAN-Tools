#ifndef UINPUT_HELPER_H
#define UINPUT_HELPER_H

// Linux Userspace Input Subsystem Helper -------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2025.11.23
//
// Description: Helper functions for using the linux userspace input subsystem (uinput).

// Includes -------------------------------------------------------------------------------------------------------------------

// C Standard Library
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

// Constants ------------------------------------------------------------------------------------------------------------------

/// @brief An alias for a uinput code.
typedef struct
{
	const char* alias;
	int code;
} uinputCodeAlias_t;

/// @brief Aliases of common key inputs.
extern const uinputCodeAlias_t UINPUT_KEY_ALIASES [];

/// @brief Number of elements in @c UINPUT_KEY_ALIASES .
extern const size_t UINPUT_KEY_ALIAS_COUNT;

/// @brief Aliases of common absolute axis inputs.
extern const uinputCodeAlias_t UINPUT_ABS_ALIASES [];

/// @brief Number of elements in @c UINPUT_ABS_ALIASES .
extern const size_t UINPUT_ABS_ALIAS_COUNT;

// Functions ------------------------------------------------------------------------------------------------------------------

/**
 * @brief Initializes userspace input. Opens the uinput file and enables the device for the specified input types.
 * @param key Indicates key inputs should be enabled.
 * @param abs Indicates absolute axis inputs should be enabled.
 * @return The file descriptor of the uinput file, if successful, -1 otherwise.
 */
int uinputInit (bool key, bool abs);

/**
 * @brief Sets up and creates a uinput device. Note this should not be called until all other uinput setup has been performed.
 * @param fd The file descriptor of the uinput file.
 * @param vendor The USB vendor ID to use.
 * @param product The USB product ID to use.
 * @param name The name to give the device.
 * @return 0 if successful, the error code otherwise.
 */
int uinputSetup (int fd, uint16_t vendor, uint16_t product, char* name);

/**
 * @brief Emits an input event.
 * @param fd The file descriptor of the uinput file.
 * @param type The type of the input event.
 * @param code The code of the input event.
 * @param value The value of the event.
 * @return 0 if successful, the error code otherwise.
 */
int uinputEmit (int fd, int type, int code, int value);

/**
 * @brief Emits the input synchronizaion event.
 * @param fd The file descriptor of the uinput file.
 * @return 0 if successful, the error code otherwise.
 */
int uinputSync (int fd);

/**
 * @brief Gets a key code from an alias.
 * @param alias The alias of the code.
 * @return The code, if successful, -1 otherwise.
 */
int uinputKeyAlias (const char* alias);

/**
 * @brief Gets an absolute axis code from an alias.
 * @param alias The alias of the code.
 * @return The code, if successful, -1 otherwise.
 */
int uinputAbsAlias (const char* alias);

/**
 * @brief Destroys and closes a uinput device.
 * @param fd The file descriptor of the uinput file.
 */
void uinputClose (int fd);

#endif // UINPUT_HELPER_H
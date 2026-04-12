#ifndef MISC_PORT_H
#define MISC_PORT_H

// C Standard Library
#include <stdio.h>

/**
 * @brief Expands an environment variable inside a string.
 * @note Only the first environment variable is expanded.
 * @param str The string to expand.
 * @return A dynamically allocated string. Must be free'd with @c free . @c NULL on failure.
 */
char* expandEnv (const char* str);

/**
 * @brief Portability for the POSIX @c mkdir function. For some reason, MINGW defines this with a different function signature
 * than POSIX does, so this function corrects that.
 * @param path The path of the directory to create.
 * @return 0 if successful, -1 otherwise and @c errno is set to indicate the errno.
 */
int mkdirPort (const char* path);

/**
 * @brief Portability for the POSIX @c fsync function. On Linux, the file descriptor of the @c file is passed directly to
 * @c fsync . On Windows, this call is ignored.
 * @param file The file to sync.
 * @return The return value of @c fsync .
 */
int fsyncPort (FILE* file);

/**
 * @brief Executes a system command from a format string. Note due to using both dynamic memory allocation and the @c system
 * system call, performance of this function is rather poor. If performance is important, @c fork and @c exec should be
 * preferred.
 * @param format The command to execute. Note this can be a format string, in which case the following arguments should be the
 * values to be inserted in place of the format specifiers.
 * @param ... The variadic arguments to insert into the format string. Same convention as the @c printf family of functions.
 * @return The same convention as the return code for @c system . -1 on error and @c errno is set.
 */
int systemf (char* format, ...);

/**
 * @brief Gets the base name of a file/directory, excluding the path.
 * @param path The path to the directory. Note this is not modified.
 * @return A dynamically allocated string indicating the name, if successful, @c NULL otherwise.
 */
char* getBaseName (char* path);

/**
 * @brief Gets the free and total storage of a file / directory.
 * @param free The variable to initialize with the amount of total storage (in bytes).
 * @param total The variable to initialize with the amount of free storage (in bytes).
 * @return 0 if successful, -1 otherwise and @c errno is set to indicate the errno.
*/
int getStorageInfo (size_t* free, size_t* total, char* dir);

/**
 * @brief Gets the temperature of the CPU. Note: this function is not thread-safe, in order to use
 * it a multithreaded context, a mutex must be employed.
 * @param temp The variable to initialize with the temperature of the CPU. Divide the result by 1000 to convert to Celsius.
 * @return 0 if successful, -1 otherwise and @c errno is set to indicate the errno.
 */
int getCpuTemperature (size_t* temp);

#endif // MISC_PORT_H
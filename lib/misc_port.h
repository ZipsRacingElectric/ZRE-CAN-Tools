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

#endif // MISC_PORT_H
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
 * @brief Gets the amount of used storage and the total storage of a directory's filesystem. This does not indicate the how
 * much storage is used by the directory itself, but rather how much storage is used in the filesystem the directory belongs
 * to.
 * @note This function is only implemented for Linux.
 * @param storageUsed Buffer to write the amount of used storage (in bytes) into.
 * @param storageTotal Buffer to write the total amount of storage (in bytes) into.
 * @return 0 if successful, -1 otherwise and @c errno is set to indicate the error code.
*/
int getStorageUtilization (size_t* storageUsed, size_t* storageTotal, char* dir);

/**
 * @brief Gets the amount of used and total memory (in bytes) of a system.
 * @note This function is only implemented for Linux.
 * @param memoryUsed Buffer to write the amount of used storage (in bytes) into.
 * @param memoryTotal Buffer to write the total amount of storage (in bytes) into.
 * @return 0 if successful, -1 otherwise and @c errno is set to indicate the error code.
*/
int getMemoryUtilization (size_t* memoryUsed, size_t* memoryTotal);

// REVIEW(Barach): Percentage
/**
 * @brief Gets the cumulative used CPU time and total CPU time.
 * Note: the difference of the current and previous values within an arbitrary interval will compute the CPU utilization of that interval.
 * @note This function is only implemented for Linux.
 * @param cpuUsed Buffer to write the cumulative used CPU time (in jiffies) into.
 * @param cpuTotal Buffer to write the cumulative total CPU time (in jiffies) into.
 * @return 0 if successful, -1 otherwise and @c errno is set to indicate the error code.
 */
int getCpuUtilization (size_t* cpuUsed, size_t* cpuTotal);

/**
 * @brief Gets the temperature of the CPU.
 * @note This function is only implemented for Linux.
 * @param cpuTemperatureValue Buffer to write the value of the CPU socket temperature into. Divide the result by 1000 to convert to Celsius.
 * @return 0 if successful, -1 otherwise and @c errno is set to indicate the error code.
 */
int getCpuTemperature (size_t* cpuTemperatureValue);

#endif // MISC_PORT_H
// DART Firmware Updating -----------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2026.03.24
//
// Description: Functions for performing firmware updates with the DART.

// Functions ------------------------------------------------------------------------------------------------------------------

/**
 * @brief Updates the DART's version of the init-system.
 * @param localPath The local path to the version of the init-system to use. Cannot be @c NULL .
 * @param sshOptions The options to pass to SSH/SCP.
 * @param host The host to connect to.
 * @return 0 if successful, -1 otherwise. Note, @c errno is not set on failure, an error message is printed instead.
 */
int updateInitSystem (char* localPath, char* sshOptions, char* host);

/**
 * @brief Updates the DART's version of ZRE-CAN-Tools.
 * @param localPath The local path to the version of ZRE-CAN-Tools to use. Cannot be @c NULL .
 * @param sshOptions The options to pass to SSH/SCP.
 * @param host The host to connect to.
 * @return 0 if successful, -1 otherwise. Note, @c errno is not set on failure, an error message is printed instead.
 */
int updateZreCantools (char* localPath, char* sshOptions, char* host);
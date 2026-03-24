// Header
#include "firmware_update.h"

// Includes
#include "debug.h"
#include "misc_port.h"

// C Standard Library
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void abortZreCantools (char* sshOptions, char* host)
{
	(void) sshOptions;
	(void) host;

	printf ("\n\nUpdate aborted. Maintenance may be required.\n\n");
}

static void recoverZreCantools (char* sshOptions, char* host)
{
	printf ("\n\nUpdate failed. Attempting recovery...\n\n");

	// Remove the failed version of ZRE-CAN-Tools.
	if (systemf ("ssh %s %s \"rm -rf /root/zre_cantools/\"", sshOptions, host) != 0)
	{
		printf ("Failed to recover firmware. Maintenance is required.\n\n");
		return;
	}

	// Restore the old version
	if (systemf ("ssh %s %s \"mv /root/zre_cantools_backup/ /root/zre_cantools/\"", sshOptions, host) != 0)
	{
		printf ("Failed to recover firmware. Maintenance is required.\n\n");
		return;
	}

	// Restart the init-system
	if (systemf ("ssh %s %s \"systemctl restart init_system\"", sshOptions, host) != 0)
	{
		printf ("Failed to recover firmware. Maintenance is required.\n\n");
		return;
	}

	printf ("Firmware recovery successful. No maintenance is required.\n\n");
}

int updateZreCantools (char* localPath, char* sshOptions, char* host)
{
	printf ("\nZRE-CAN-Tools Firmware Update --------------------------------------------------\n\n");

	// Stop the init-system
	if (systemf ("ssh %s %s \"systemctl stop init_system\"", sshOptions, host) != 0)
	{
		abortZreCantools (sshOptions, host);
		return -1;
	}

	// Backup the DART's version of ZRE-CAN-Tools
	if (systemf ("ssh %s %s \"mv /root/zre_cantools/ /root/zre_cantools_backup/\"", sshOptions, host) != 0)
	{
		abortZreCantools (sshOptions, host);
		return -1;
	}

	// Create the destination directory
	if (systemf ("ssh %s %s \"mkdir -p /root/zre_cantools\"", sshOptions, host) != 0)
	{
		recoverZreCantools (sshOptions, host);
		return -1;
	}

	// Copy the target version of ZRE-CAN-Tools onto the DART
	bool successful = true;
	successful &= systemf ("scp %s -r %s/config/ %s:/root/zre_cantools/", sshOptions, localPath, host) == 0;
	successful &= systemf ("scp %s -r %s/src/ %s:/root/zre_cantools/", sshOptions, localPath, host) == 0;
	successful &= systemf ("scp %s -r %s/lib/ %s:/root/zre_cantools/", sshOptions, localPath, host) == 0;
	successful &= systemf ("scp %s %s/include.mk %s:/root/zre_cantools/", sshOptions, localPath, host) == 0;
	successful &= systemf ("scp %s %s/makefile %s:/root/zre_cantools/", sshOptions, localPath, host) == 0;
	if (!successful)
	{
		recoverZreCantools (sshOptions, host);
		return -1;
	}

	// Compile ZRE-CAN-Tools on the DART
	if (systemf ("ssh %s %s \"make -C /root/zre_cantools/\"", sshOptions, host) != 0)
	{
		recoverZreCantools (sshOptions, host);
		return -1;
	}

	// Restart the init-system
	if (systemf ("ssh %s %s \"systemctl restart init_system\"", sshOptions, host) != 0)
	{
		recoverZreCantools (sshOptions, host);
		return -1;
	}

	// Delete the old version of ZRE-CAN-Tools
	if (systemf ("ssh %s %s \"rm -rf /root/zre_cantools_backup/\"", sshOptions, host) != 0)
	{
		recoverZreCantools (sshOptions, host);
		return -1;
	}

	printf ("\n\nFirmware Update Successful.\n\n");
	return 0;
}

static void abortInitSystem (char* sshOptions, char* host)
{
	(void) sshOptions;
	(void) host;

	printf ("\n\nUpdate aborted. Maintenance may be required.\n\n");
}

static void recoverInitSystem (char* sshOptions, char* host)
{
	printf ("\n\nUpdate failed. Attempting recovery...\n\n");

	// Remove the failed version of the init-system.
	if (systemf ("ssh %s %s \"rm -rf /root/init_system/\"", sshOptions, host) != 0)
	{
		printf ("Failed to recover firmware. Maintenance is required.\n\n");
		return;
	}

	// Restore the old version
	if (systemf ("ssh %s %s \"mv /root/init_system_backup/ /root/init_system/\"", sshOptions, host) != 0)
	{
		printf ("Failed to recover firmware. Maintenance is required.\n\n");
		return;
	}

	// Restart the init-system
	if (systemf ("ssh %s %s \"systemctl restart init_system\"", sshOptions, host) != 0)
	{
		printf ("Failed to recover firmware. Maintenance is required.\n\n");
		return;
	}

	printf ("Firmware recovery successful. No maintenance is required.\n\n");
}

int updateInitSystem (char* localPath, char* sshOptions, char* host)
{
	printf ("\nInit-system Firmware Update ----------------------------------------------------\n\n");

	// Validate the name of the target init-system directory

	char* baseDirName = getDirName (localPath);
	if (baseDirName == NULL)
	{
		errorPrintf ("Failed to get target init-system directory name");
		printf ("\n\nUpdate aborted. No maintenance is required.\n\n");
		return -1;
	}

	if (strcmp (baseDirName, "init_system") != 0)
	{
		fprintf (stderr, "Target init-system directory must be named \"init_system\".\n");
		printf ("\n\nUpdate aborted. No maintenance is required.\n\n");
		return -1;
	}

	free (baseDirName);

	// Stop the init-system
	if (systemf ("ssh %s %s \"systemctl stop init_system\"", sshOptions, host) != 0)
	{
		abortInitSystem (sshOptions, host);
		return -1;
	}

	// Backup the DART's version of the init-system
	if (systemf ("ssh %s %s \"mv /root/init_system/ /root/init_system_backup/\"", sshOptions, host) != 0)
	{
		abortInitSystem (sshOptions, host);
		return -1;
	}

	// Copy the target version of the init-system onto the DART
	if (systemf ("scp %s -r %s %s:/root/", sshOptions, localPath, host) != 0)
	{
		recoverInitSystem (sshOptions, host);
		return -1;
	}

	// Compile the init_system on the DART
	if (systemf ("ssh %s %s \"make -C /root/init_system/\"", sshOptions, host) != 0)
	{
		recoverInitSystem (sshOptions, host);
		return -1;
	}

	// Restart the init-system
	if (systemf ("ssh %s %s \"systemctl restart init_system\"", sshOptions, host) != 0)
	{
		recoverInitSystem (sshOptions, host);
		return -1;
	}

	// Delete the old version of the init-system
	if (systemf ("ssh %s %s \"rm -rf /root/init_system_backup/\"", sshOptions, host) != 0)
	{
		recoverInitSystem (sshOptions, host);
		return -1;
	}

	printf ("\n\nFIRMWARE UPDATE SUCCESSFUL.\n\n");
	return 0;
}

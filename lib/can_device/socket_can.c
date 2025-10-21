// Header
#include "socket_can.h"

// Includes
#include "can_device.h"
#include "error_codes.h"

#if defined (__unix__)

// SocketCAN Libraries
#include <linux/can.h>
#include <linux/can/raw.h>

// POSIX Libraries
#include <fcntl.h>
#include <net/if.h>
#include <stdint.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <unistd.h>

#endif // __unix__

// C Standard Libraries
#include <errno.h>
#include <string.h>
#include <stdlib.h>

// Datatypes ------------------------------------------------------------------------------------------------------------------

typedef struct
{
	// TODO(Barach): Docs
	canDeviceVmt_t vmt;
	const char* name;
	int descriptor;
} socketCan_t;

// Functions ------------------------------------------------------------------------------------------------------------------

bool socketCanNameDomain (const char* name)
{
	if (strncmp("can", name, strlen ("can")) == 0)
		return true;

	if (strncmp("vcan", name, strlen ("vcan")) == 0)
		return true;

	return false;
}

canDevice_t* socketCanInit (const char* name)
{
	#if defined (__unix__)

	// Create the socket using the CAN protocol family and the raw CAN protol
	int descriptor = socket (PF_CAN, SOCK_RAW, CAN_RAW);
	if (descriptor == -1)
		return NULL;

	// Create the interface structure
	struct ifreq interface;

	// Set interface name, set terminating character if name exceeds bounds
	strncpy (interface.ifr_name, name, IFNAMSIZ - 1);
	interface.ifr_name[IFNAMSIZ - 1] = '\0';

	// Get the interface index from its name
	if (ioctl (descriptor, SIOCGIFINDEX, &interface) == -1)
	{
		close (descriptor);
		return NULL;
	}

	// Allocate the CAN socket address
	struct sockaddr_can address =
	{
		.can_family		= AF_CAN,					// CAN Address family
		.can_ifindex	= interface.ifr_ifindex,	// CAN Interface index
	};

	// Bind the socket to the specified address
	if (bind (descriptor, (struct sockaddr*) (&address), (socklen_t) (sizeof(address))) == -1)
	{
		close (descriptor);
		return NULL;
	}

	// Device must be dynamically allocated
	socketCan_t* device = malloc (sizeof (socketCan_t));

	// Setup the device's VMT
	device->vmt.transmit	= socketCanTransmit;
	device->vmt.receive 	= socketCanReceive;
	device->vmt.flushRx 	= socketCanFlushRx;
	device->vmt.setTimeout	= socketCanSetTimeout;

	// Internal housekeeping
	device->descriptor = descriptor;
	device->name = name;

	// Success
	return (canDevice_t*) device;

	#else // __unix__

	(void) name;

	errno = ERRNO_OS_NOT_SUPPORTED;
	return NULL;

	#endif // __unix__
}

int socketCanDealloc (void* device)
{
	// TODO(Barach):
	// #if defined (__unix__)

	// socketCan_t* socket = device;

	// // Close the socket.
	// if (close (socket->descriptor) != 0)
	// 	return errno;

	// // Free the device's memory.
	// free (socket);

	// return 0;

	// #else // __unix__

	(void) device;
	errno = ERRNO_OS_NOT_SUPPORTED;
	return errno;

	// #endif // __unix__
}

int socketCanTransmit (void* device, canFrame_t* frame)
{
	#if defined (__unix__)

	socketCan_t* socket = device;

	// Create the can_frame struct
	struct can_frame socketFrame =
	{
		.can_dlc = frame->dlc,
		.can_id = frame->id,
	};
	memcpy (socketFrame.data, frame->data, frame->dlc);

	// Transmit the frame
	int code = write (socket->descriptor, &socketFrame, sizeof (struct can_frame));
	if(code < (long int) sizeof (struct can_frame))
		return errno;

	// Success
	return 0;

	#else // __unix__

	(void) device;
	(void) frame;

	errno = ERRNO_OS_NOT_SUPPORTED;
	return errno;

	#endif // __unix__
}

int socketCanReceive (void* device, canFrame_t* frame)
{
	#if defined (__unix__)

	socketCan_t* socket = device;

	struct can_frame socketFrame;

	// Read the frame
	int code = read (socket->descriptor, &socketFrame, sizeof (struct can_frame));
	if (code < (long int) sizeof (struct can_frame))
		return errno;

	// TODO(Barach): Check status for return value?
	// Mask out status bits
	frame->id = socketFrame.can_id & 0x1FFFFFFF;

	// Convert to canFrame_t
	frame->dlc = socketFrame.can_dlc;
	memcpy (frame->data, socketFrame.data, socketFrame.can_dlc);

	// Success
	return 0;

	#else // __unix__

	(void) device;
	(void) frame;

	errno = ERRNO_OS_NOT_SUPPORTED;
	return errno;

	#endif // __unix__
}

int socketCanFlushRx (void* device)
{
	#if defined (__unix__)

	socketCan_t* socket = device;

	// Get the sockets flags.
	int flags = fcntl (socket->descriptor, F_GETFL);
	if (flags == -1)
		return errno;

	// Make the socket nonblocking.
	flags |= O_NONBLOCK;
	if (fcntl (socket->descriptor, F_SETFL, flags) != 0)
		return errno;

	// Read all available data from the socket.
	struct can_frame frame;
	while (read (socket->descriptor, &frame, sizeof (struct can_frame)) == sizeof (struct can_frame));

	// Make the socket blocking again.
	flags &= ~O_NONBLOCK;
	if (fcntl (socket->descriptor, F_SETFL, flags) != 0)
		return errno;

	return 0;

	#else // __unix__

	(void) device;

	errno = ERRNO_OS_NOT_SUPPORTED;
	return errno;

	#endif // __unix__
}

int socketCanSetTimeout (void* device, unsigned long timeoutMs)
{
	#if defined (__unix__)

	socketCan_t* socket = device;

	struct timeval timeout =
	{
		.tv_sec = timeoutMs / 1000,
		.tv_usec = (timeoutMs % 1000) * 1000
	};

	if (setsockopt (socket->descriptor, SOL_SOCKET, SO_RCVTIMEO, (void*) &timeout, (socklen_t) sizeof (timeout)) != 0)
		return errno;

	return 0;

	#else // __unix__

	(void) device;
	(void) timeoutMs;

	errno = ERRNO_OS_NOT_SUPPORTED;
	return errno;

	#endif // __unix__
}
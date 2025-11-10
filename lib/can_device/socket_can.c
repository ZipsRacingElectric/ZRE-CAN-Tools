// Header
#include "socket_can.h"

// Includes
#include "can_device.h"
#include "error_codes.h"

#if defined (__unix__)

// SocketCAN Libraries
#include <linux/can.h>
#include <linux/can/error.h>
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
	canBaudrate_t baudrate;
} socketCan_t;

// Functions ------------------------------------------------------------------------------------------------------------------

#if defined (__unix__)

static int getErrorCode (struct can_frame* frame)
{
	// Check for protocol error
	if (frame->can_id & CAN_ERR_PROT)
	{
		uint8_t typeFlags = frame->data [2];
		uint8_t locFlags = frame->data [3];

		// Form error
		if (typeFlags & CAN_ERR_PROT_FORM)
			return ERRNO_CAN_DEVICE_FORM_ERROR;

		// Bit stuff error
		if (typeFlags & CAN_ERR_PROT_STUFF)
			return ERRNO_CAN_DEVICE_BIT_STUFF_ERROR;

		// CRC error
		if (locFlags & CAN_ERR_PROT_LOC_CRC_SEQ || locFlags & CAN_ERR_PROT_LOC_CRC_DEL)
			return ERRNO_CAN_DEVICE_CRC_ERROR;

		// Bit error
		if (typeFlags & CAN_ERR_PROT_BIT)
			return ERRNO_CAN_DEVICE_BIT_ERROR;

		// ACK error
		if (locFlags & CAN_ERR_PROT_LOC_ACK || locFlags & CAN_ERR_PROT_LOC_ACK_DEL)
			return ERRNO_CAN_DEVICE_ACK_ERROR;
	}

	// Bus-off error
	if ((frame->can_id & CAN_ERR_BUSOFF) == CAN_ERR_BUSOFF)
		return ERRNO_CAN_DEVICE_BUS_OFF;

	// Unspecified error
	return ERRNO_CAN_DEVICE_UNSPEC_ERROR;
}

#endif // __unix__

bool socketCanNameDomain (const char* name)
{
	if (strncmp("can", name, strlen ("can")) == 0)
		return true;

	if (strncmp("vcan", name, strlen ("vcan")) == 0)
		return true;

	return false;
}

canDevice_t* socketCanInit (const char* name, canBaudrate_t baudrate)
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

	// Set the socket's error filter to include all error types.
	can_err_mask_t errorMask = CAN_ERR_MASK;
	if (setsockopt(descriptor, SOL_CAN_RAW, CAN_RAW_ERR_FILTER, &errorMask, sizeof (errorMask)) != 0)
	{
		close (descriptor);
		return NULL;
	}

	// Device must be dynamically allocated
	socketCan_t* device = malloc (sizeof (socketCan_t));

	// Setup the device's VMT
	device->vmt.transmit		= socketCanTransmit;
	device->vmt.receive 		= socketCanReceive;
	device->vmt.flushRx 		= socketCanFlushRx;
	device->vmt.setTimeout		= socketCanSetTimeout;
	device->vmt.getBaudrate		= socketCanGetBaudrate;
	device->vmt.getDeviceName	= socketCanGetDeviceName;
	device->vmt.getDeviceType	= socketCanGetDeviceType;
	device->vmt.dealloc			= socketCanDealloc;

	// Internal housekeeping
	device->descriptor = descriptor;
	device->name = name;
	device->baudrate = baudrate;

	// Success
	return (canDevice_t*) device;

	#else // __unix__

	(void) name;

	errno = ERRNO_OS_NOT_SUPPORTED;
	return NULL;

	#endif // __unix__
}

void socketCanDealloc (void* device)
{
	#if defined (__unix__)

	socketCan_t* sock = device;

	// Close the socket.
	close (sock->descriptor);

	// Free the device's memory.
	free (sock);

	#endif // __unix__
}

int socketCanTransmit (void* device, canFrame_t* frame)
{
	#if defined (__unix__)

	socketCan_t* sock = device;

	// Convert to a SocketCAN frame
	struct can_frame socketFrame =
	{
		.can_dlc = frame->dlc,
		.can_id = frame->id | (frame->ide ? CAN_EFF_FLAG : 0) | (frame->rtr ? CAN_RTR_FLAG : 0),
	};
	memcpy (socketFrame.data, frame->data, frame->dlc);

	// Transmit the frame
	int code = write (sock->descriptor, &socketFrame, sizeof (struct can_frame));
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

	socketCan_t* sock = device;

	struct can_frame socketFrame;

	// Read the frame
	int code = read (sock->descriptor, &socketFrame, sizeof (struct can_frame));
	if (code < (long int) sizeof (struct can_frame))
	{
		// Translate the "would block" error into a timeout error.
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			errno = ERRNO_CAN_DEVICE_TIMEOUT;

		return errno;
	}

	// Convert back from the SocketCAN frame
	frame->id = socketFrame.can_id & CAN_EFF_MASK;
	frame->ide = (socketFrame.can_id & CAN_EFF_FLAG) == CAN_EFF_FLAG;
	frame->rtr = (socketFrame.can_id & CAN_RTR_FLAG) == CAN_RTR_FLAG;
	frame->dlc = socketFrame.can_dlc;
	memcpy (frame->data, socketFrame.data, socketFrame.can_dlc);

	// Check for error flags, if set, handle the error frame
	if (socketFrame.can_id & CAN_ERR_FLAG)
	{
		errno = getErrorCode (&socketFrame);
		return errno;
	}

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

	socketCan_t* sock = device;

	// Get the sockets flags.
	int flags = fcntl (sock->descriptor, F_GETFL);
	if (flags == -1)
		return errno;

	// Make the socket nonblocking.
	flags |= O_NONBLOCK;
	if (fcntl (sock->descriptor, F_SETFL, flags) != 0)
		return errno;

	// Read all available data from the socket.
	struct can_frame frame;
	while (read (sock->descriptor, &frame, sizeof (struct can_frame)) == sizeof (struct can_frame));

	// Make the socket blocking again.
	flags &= ~O_NONBLOCK;
	if (fcntl (sock->descriptor, F_SETFL, flags) != 0)
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

	socketCan_t* sock = device;

	struct timeval timeout =
	{
		.tv_sec = timeoutMs / 1000,
		.tv_usec = (timeoutMs % 1000) * 1000
	};

	if (setsockopt (sock->descriptor, SOL_SOCKET, SO_RCVTIMEO, (void*) &timeout, (socklen_t) sizeof (timeout)) != 0)
		return errno;

	return 0;

	#else // __unix__

	(void) device;
	(void) timeoutMs;

	errno = ERRNO_OS_NOT_SUPPORTED;
	return errno;

	#endif // __unix__
}

canBaudrate_t socketCanGetBaudrate (void* device)
{
	return ((socketCan_t*) device)->baudrate;
}

const char* socketCanGetDeviceName (void* device)
{
	return ((socketCan_t*) device)->name;
}

const char* socketCanGetDeviceType (void)
{
	return "SocketCAN";
}
// Header
#include "can_socket.h"

// SocketCAN Libraries
#include <linux/can.h>
#include <linux/can/raw.h>

// POSIX Libraries
#include <net/if.h>
#include <stdint.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <unistd.h>

// C Standard Libraries
#include <errno.h>
#include <math.h>
#include <string.h>

// Debugging ------------------------------------------------------------------------------------------------------------------

#if CAN_SOCKET_DEBUG
	#include <stdio.h>
	#define DEBUG_PRINTF(...) printf(__VA_ARGS__)
#else
	#define DEBUG_PRINTF(...) while (false)
#endif // CAN_SOCKET_DEBUG

// CAN Socket -----------------------------------------------------------------------------------------------------------------

bool canSocketInit (canSocket_t* canSocket, const char* deviceName)
{
	int code;

	canSocket->name = deviceName;

	// Create the socket using the CAN protocol family and the raw CAN protol
	canSocket->descriptor = socket (PF_CAN, SOCK_RAW, CAN_RAW);
	if (canSocket->descriptor == -1)
	{
		code = errno;
		DEBUG_PRINTF ("Failed to create CAN socket for interface '%s': %s.\n", deviceName, strerror(code));
		return false;
	}

	// Create the interface structure
	struct ifreq interface;
	
	// Set interface name, set terminating character if name exceeds bounds
	strncpy (interface.ifr_name, deviceName, IFNAMSIZ - 1);
	interface.ifr_name[IFNAMSIZ - 1] = '\0';

	// Get the interface index from its name
	code = ioctl (canSocket->descriptor, SIOCGIFINDEX, &interface);
	if (code == -1)
	{
		code = errno;
		DEBUG_PRINTF ("Failed to get index of CAN device '%s': %s\n", deviceName, strerror (code));
		return false;
	}

	// Allocate the CAN socket address
	struct sockaddr_can address =
	{
		.can_family		= AF_CAN,					// CAN Address family
		.can_ifindex	= interface.ifr_ifindex,	// CAN Interface index
	};

	// Bind the socket to the specified address
	code = bind (canSocket->descriptor, (struct sockaddr*) (&address), (socklen_t) (sizeof(address)));
	if(code == -1)
	{
		code = errno;
		DEBUG_PRINTF ("Failed to bind CAN socket '%s' to index %i: %s\n", deviceName, interface.ifr_ifindex,
			strerror (code));
		return false;
	}

	return true;
}

bool canSocketTransmit (canSocket_t* canSocket, struct can_frame* frame)
{
	int code = write (canSocket->descriptor, frame, sizeof (struct can_frame));
	if(code < (long int) sizeof (struct can_frame))
	{
		code = errno;
		DEBUG_PRINTF ("Failed to send CAN message: %s\n", strerror(code));
		return false;
	}

	return true;
}

bool canSocketReceive (canSocket_t* canSocket, struct can_frame* frame)
{
	// Read message
	int code = read (canSocket->descriptor, frame, sizeof (struct can_frame));

	if (code < (long long int) sizeof (struct can_frame))
	{
		code = errno;

		// Check for timeout
		// TODO(Barach): Timeout handling
		// if(code == EAGAIN || code == EWOULDBLOCK || code == EINPROGRESS)

		DEBUG_PRINTF ("Failed to read CAN message: %s\n", strerror (code));
		return false;
	}

	return true;
}

uint64_t signalEncode (canSignal_t* signal, float value)
{
	uint64_t buffer = roundf (value - signal->offset) / signal->scaleFactor;
	buffer &= signal->bitmask;
	buffer <<= signal->bitPosition;
	return buffer;
}

float signalDecode (canSignal_t* signal, uint64_t payload)
{
	payload >>= signal->bitPosition;
	payload &= signal->bitmask;

	// Perform sign extension
	bool negative = payload >> (signal->bitLength - 1) && signal->signedness;
	if (negative)
		payload |= ((uint64_t) -1) - signal->bitmask;

	float value = (signal->signedness ? (float) ((int64_t) payload) : (float) payload);
	return value * signal->scaleFactor + signal->offset;
}

// Standard I/O ---------------------------------------------------------------------------------------------------------------

void framePrint (struct can_frame* frame)
{
	printf ("%X\t[%u]\t", frame->can_id, frame->can_dlc);
	for (uint8_t index = 0; index < frame->can_dlc; ++index)
		printf ("%2X ", frame->data [index]);
	printf ("\n");
}

void messagePrint (canMessage_t* message, struct can_frame* frame)
{
	printf ("- Message %s (0x%X) -\n", message->name, message->id);

	for (size_t index = 0; index < message->signalCount; ++index)
		signalPrint (&message->signals [index], *(uint64_t*) frame->data);
}

struct can_frame messagePrompt (canMessage_t* message)
{
	struct can_frame frame =
	{
		.can_id		= message->id,
		.can_dlc	= message->dlc,
	};

	uint64_t* payload = (uint64_t*) frame.data;

	printf ("- Message %s (0x%X) -\n", message->name, message->id);

	for (size_t index = 0; index < message->signalCount; ++index)
		*payload |= signalPrompt (message->signals + index);

	return frame;
}

void signalPrint (canSignal_t* signal, uint64_t payload)
{
	float value = signalDecode (signal, payload);
	printf ("%s: %f\n", signal->name, value);
}

uint64_t signalPrompt (canSignal_t* signal)
{
	while (true)
	{
		printf ("%s: ", signal->name);

		float value;
		if (fscanf (stdin, "%f%*1[\n]", &value) == 1)
			return signalEncode (signal, value);

		printf ("Invalid value.\n");
	}
}
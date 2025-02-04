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
#include <stdio.h>
#include <string.h>

// Debugging ------------------------------------------------------------------------------------------------------------------

#if CAN_SOCKET_DEBUG
	#include <stdio.h>
	#define DEBUG_PRINTF(...) printf(__VA_ARGS__)
#else
	#define DEBUG_PRINTF(...) while (false)
#endif // CAN_SOCKET_DEBUG

// CAN Socket -----------------------------------------------------------------------------------------------------------------

int canSocketInit (canSocket_t* canSocket, const char* deviceName)
{
	canSocket->name = deviceName;

	// Create the socket using the CAN protocol family and the raw CAN protol
	canSocket->descriptor = socket (PF_CAN, SOCK_RAW, CAN_RAW);
	if (canSocket->descriptor == -1)
		return errno;

	// Create the interface structure
	struct ifreq interface;

	// Set interface name, set terminating character if name exceeds bounds
	strncpy (interface.ifr_name, deviceName, IFNAMSIZ - 1);
	interface.ifr_name[IFNAMSIZ - 1] = '\0';

	// Get the interface index from its name
	if (ioctl (canSocket->descriptor, SIOCGIFINDEX, &interface) == -1)
		return errno;

	// Allocate the CAN socket address
	struct sockaddr_can address =
	{
		.can_family		= AF_CAN,					// CAN Address family
		.can_ifindex	= interface.ifr_ifindex,	// CAN Interface index
	};

	// Bind the socket to the specified address
	if (bind (canSocket->descriptor, (struct sockaddr*) (&address), (socklen_t) (sizeof(address))) == -1)
		return errno;

	return 0;
}

int canSocketTransmit (canSocket_t* canSocket, struct can_frame* frame)
{
	int code = write (canSocket->descriptor, frame, sizeof (struct can_frame));
	if(code < (long int) sizeof (struct can_frame))
		return errno;

	return 0;
}

int canSocketReceive (canSocket_t* canSocket, struct can_frame* frame)
{
	int code = read (canSocket->descriptor, frame, sizeof (struct can_frame));
	if (code < (long int) sizeof (struct can_frame))
		return errno;

	return 0;
}

int canSocketSetTimeout (canSocket_t* canSocket, unsigned long timeMs)
{
	struct timeval timeout =
	{
		.tv_sec = 0,
		.tv_usec = 0
	};

	if (timeMs != 0)
	{
		timeout.tv_usec = (timeMs % 1000) * 1000;
		timeout.tv_sec = timeMs / 1000;
	}

	if (setsockopt(canSocket->descriptor, SOL_SOCKET, SO_RCVTIMEO, (void*) &timeout, (socklen_t) sizeof (timeout)) != 0)
		return errno;

	return 0;
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

void framePrint (FILE* stream, struct can_frame* frame)
{
	fprintf (stream, "%X\t[%u]\t", frame->can_id, frame->can_dlc);
	for (uint8_t index = 0; index < frame->can_dlc; ++index)
		fprintf (stream, "%2X ", frame->data [index]);
	fprintf (stream, "\n");
}

void messagePrint (FILE* stream, canMessage_t* message, struct can_frame* frame)
{
	fprintf (stream, "- Message %s (0x%X) -\n", message->name, message->id);

	for (size_t index = 0; index < message->signalCount; ++index)
		signalPrint (stream, &message->signals [index], *(uint64_t*) frame->data);
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

void signalPrint (FILE* stream, canSignal_t* signal, uint64_t payload)
{
	float value = signalDecode (signal, payload);
	fprintf (stream, "%s: %f\n", signal->name, value);
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
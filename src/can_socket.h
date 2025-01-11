#ifndef CAN_SOCKET_H
#define CAN_SOCKET_H

// CAN Socket -----------------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2023.07.08
//
// Description: An interface for CAN devices based on the Linux SocketCAN implementation.
//
// To do:
// - Socket timeout settings and handling.
//
// References:
// - https://www.kernel.org/doc/html/latest/networking/can.html
// - https://en.wikipedia.org/wiki/SocketCAN
// - https://www.man7.org/linux/man-pages/man2/socket.2.html
// - https://www.man7.org/linux/man-pages/man7/netdevice.7.html
// - https://www.gnu.org/software/libc/manual/html_node/Interface-Naming.html
// - https://www.can-cia.org/fileadmin/resources/documents/proceedings/2012_hartkopp.pdf
// - https://github.com/linux-can/can-utils/tree/master

// Includes -------------------------------------------------------------------------------------------------------------------

// SocketCAN Libraries
#include <linux/can.h>

// C Standard Library
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

// Debugging ------------------------------------------------------------------------------------------------------------------

#define CAN_SOCKET_DEBUG 1

// Datatypes ------------------------------------------------------------------------------------------------------------------

struct canSocket
{
	const char* name;
	int descriptor;
};

typedef struct canSocket canSocket_t;

// CAN Signal forward declaration
struct canSignal;
typedef struct canSignal canSignal_t;

/**
 * @brief Structure representing a message broadcast on a CAN bus.
 */
struct canMessage
{
	canSignal_t*	signals;
	size_t			signalCount;
	char*			name;
	uint16_t		id;
	uint8_t			dlc;
};

typedef struct canMessage canMessage_t;

/**
 * @brief Structure representing a signal present in a CAN message.
 */
struct canSignal
{
	canMessage_t*	message;
	char*			name;
	uint8_t			bitPosition;
	uint8_t			bitLength;
	float			scaleFactor;
	float			offset;
	bool			signedness;
	bool			endianness;
	uint64_t		bitmask;
};

// Functions ------------------------------------------------------------------------------------------------------------------

/**
 * @brief Creates and opens a CAN socket bound to the specified device interface.
 * @param canSocket The socket structure to initialize.
 * @param deviceName The name of the device's interface to bind to.
 * @return True if successful, false otherwise.
 */
bool canSocketInit (canSocket_t* canSocket, const char* deviceName);

/**
 * @brief Transmits the specified frame on a socket.
 * @param canSocket The socket to transmit from.
 * @param frame The CAN frame to transmit.
 * @return True if successful, false otherwise.
 */
bool canSocketTransmit (canSocket_t* canSocket, struct can_frame* frame);

/**
 * @brief Receives the next frame from a socket.
 * @param canSocket The socket to receive from.
 * @param frame Written to contain the next received frame.
 * @return True if successful, false otherwise.
 */
bool canSocketReceive (canSocket_t* canSocket, struct can_frame* frame);

/**
 * @brief Sets the timeout period of a socket.
 * @param canSocket The socket to modify.
 * @param timeMs The amount of time to timeout after, in milliseconds. Use 0 to disable timeouts.
 * @return True if successful, false otherwise.
 */
bool canSocketSetTimeout (canSocket_t* canSocket, unsigned long timeMs);

/**
 * @brief Encodes the specified signal's data into a payload.
 * @param signal The signal to encode.
 * @param value The value of the signal.
 * @return The encoded payload.
 */
uint64_t signalEncode (canSignal_t* signal, float value);

/**
 * @brief Decodes the specified signal from a payload.
 * @param signal The signal to decoded.
 * @param payload The payload to decode from.
 * @return The decoded value.
 */
float signalDecode (canSignal_t* signal, uint64_t payload);

// Standard I/O ---------------------------------------------------------------------------------------------------------------

/**
 * @brief Prints the raw data of a CAN frame.
 * @param frame The frame to print.
 */
void framePrint (struct can_frame* frame);

/**
 * @brief Prints the decoded data of a CAN frame.
 * @param message The message to print.
 * @param frame The frame to decode from.
 */
void messagePrint (canMessage_t* message, struct can_frame* frame);

/**
 * @brief Prompts the user to input data for the given message.
 * @param message The message to prompt for.
 * @return A frame encoding the message's data.
 */
struct can_frame messagePrompt (canMessage_t* message);

/**
 * @brief Prints the decoded signal's data.
 * @param signal The signal to print.
 * @param payload The payload to decode from.
 */
void signalPrint (canSignal_t* signal, uint64_t payload);

/**
 * @brief Prompts the user to input a value for the given signal.
 * @param signal The signal to prompt for.
 * @return The encoded payload.
 */
uint64_t signalPrompt (canSignal_t* signal);

#endif // CAN_SOCKET_H
// Header
#include "can_eeprom_operations.h"

// Includes
#include "error_codes.h"

// C Standard Library
#include <errno.h>
#include <string.h>
#include <sys/time.h>

// Macros / Constants ---------------------------------------------------------------------------------------------------------

#define EEPROM_COMMAND_MESSAGE_READ_NOT_WRITE(rnw)			(((uint16_t) (rnw))	<< 0)
#define EEPROM_COMMAND_MESSAGE_DATA_NOT_VALIDATION(dnv)		(((uint16_t) (dnv))	<< 1)
#define EEPROM_COMMAND_MESSAGE_IS_VALID(iv)					(((uint16_t) (iv))	<< 2)
#define EEPROM_COMMAND_MESSAGE_DATA_COUNT(dc)				(((uint16_t) ((dc) - 1)) << 2)

#define EEPROM_RESPONSE_MESSAGE_READ_NOT_WRITE(word)		(((word) & 0b00000001) == 0b00000001)
#define EEPROM_RESPONSE_MESSAGE_DATA_NOT_VALIDATION(word)	(((word) & 0b00000010) == 0b00000010)
#define EEPROM_RESPONSE_MESSAGE_IS_VALID(word)				(((word) & 0b00000100) == 0b00000100)
#define EEPROM_RESPONSE_MESSAGE_DATA_COUNT(word)			((((word) & 0b00001100) >> 2) + 1)

#define RESPONSE_ATTEMPT_COUNT 7
static const struct timeval RESPONSE_ATTEMPT_TIMEOUT =
{
	.tv_sec		= 0,
	.tv_usec	= 200000
};

// Function Prototypes --------------------------------------------------------------------------------------------------------

/// @brief Encodes the command frame for a data write operation.
struct can_frame writeMessageEncode (uint16_t canId, uint16_t address, uint8_t count, void* buffer);

/// @brief Encodes the command frame for a data read operation.
struct can_frame readMessageEncode (uint16_t canId, uint16_t address, uint8_t count);

/// @brief Parses a CAN frame to check whether it is the response to a data read command.
int readMessageParse (uint16_t canId, struct can_frame* frame, uint16_t address, uint8_t count, void* buffer);

/// @brief Performs a single write operation on the EEPROM.
int writeSingle (uint16_t canId, canSocket_t* socket, uint16_t address, uint8_t count, void* buffer);

/// @brief Performs a single read operation on the EEPROM.
int readSingle (uint16_t canId, canSocket_t* socket, uint16_t address, uint8_t count, void* buffer);

/// @brief Encodes the command frame for a validation write message.
struct can_frame validateMessageEncode (uint16_t canId, bool isValid);

/// @brief Encodes the command frame for a validation read message.
struct can_frame isValidMessageEncode (uint16_t canId);

/// @brief Parses a CAN frame to check whether it is the response to a validation read command.
int isValidMessageParse (uint16_t canId, struct can_frame* frame, bool* isValid);

// Functions ------------------------------------------------------------------------------------------------------------------

int canEepromWrite (uint16_t canId, canSocket_t* socket, uint16_t address, uint16_t count, void* buffer)
{
	// Write all of the full messages
	while (count > 4)
	{
		if (writeSingle (canId, socket, address, 4, buffer) != 0)
			return errno;

		buffer += 4;
		address += 4;
		count -= 4;
	}

	// Write the last message
	return writeSingle (canId, socket, address, count, buffer);
}

int canEepromRead (uint16_t canId, canSocket_t* socket, uint16_t address, uint16_t count, void* buffer)
{
	// Read all of the full messages
	while (count > 4)
	{
		if (readSingle (canId, socket, address, 4, buffer) != 0)
			return errno;

		buffer += 4;
		address += 4;
		count -= 4;
	}

	// Read the last message
	return readSingle (canId, socket, address, count, buffer);
}

int canEepromValidate (uint16_t canId, canSocket_t* socket, bool isValid)
{
	struct can_frame commandFrame = validateMessageEncode (canId, isValid);

	bool readIsValid;
	for (uint8_t attempt = 0; attempt < RESPONSE_ATTEMPT_COUNT; ++attempt)
	{
		canSocketTransmit (socket, &commandFrame);
		if (canEepromIsValid (canId, socket, &readIsValid) != 0)
			return errno;

		if (readIsValid == isValid)
			return 0;
	}

	errno = ERRNO_CAN_EEPROM_VALIDATE_TIMEOUT;
	return errno;
}

int canEepromIsValid (uint16_t canId, canSocket_t* socket, bool* isValid)
{
	struct can_frame commandFrame = isValidMessageEncode (canId);

	for (uint8_t attempt = 0; attempt < RESPONSE_ATTEMPT_COUNT; ++attempt)
	{
		if (canSocketTransmit (socket, &commandFrame) != 0)
			return errno;

		struct timeval deadline;
		struct timeval timeCurrent;
		gettimeofday (&timeCurrent, NULL);
		timeradd (&timeCurrent, &RESPONSE_ATTEMPT_TIMEOUT, &deadline);

		while (timercmp (&timeCurrent, &deadline, <))
		{
			gettimeofday (&timeCurrent, NULL);

			struct can_frame response;
			if (canSocketReceive (socket, &response) != 0)
				continue;

			if (isValidMessageParse (canId, &response, isValid) == 0)
				return 0;
		}
	}

	errno = ERRNO_CAN_EEPROM_IS_VALID_TIMEOUT;
	return errno;
}

// Private Functions ----------------------------------------------------------------------------------------------------------

struct can_frame writeMessageEncode (uint16_t canId, uint16_t address, uint8_t count, void* buffer)
{
	uint16_t instruction = EEPROM_COMMAND_MESSAGE_READ_NOT_WRITE (false)
		| EEPROM_COMMAND_MESSAGE_DATA_NOT_VALIDATION (true)
		| EEPROM_COMMAND_MESSAGE_DATA_COUNT (count);

	struct can_frame frame =
	{
		.can_id		= canId,
		.can_dlc	= 8,
		.data		=
		{
			instruction,
			instruction >> 8,
			address,
			address >> 8
		}
	};
	memcpy (frame.data + 4, buffer, count);

	return frame;
}

struct can_frame readMessageEncode (uint16_t canId, uint16_t address, uint8_t count)
{
	uint16_t instruction = EEPROM_COMMAND_MESSAGE_READ_NOT_WRITE (true)
		| EEPROM_COMMAND_MESSAGE_DATA_NOT_VALIDATION (true)
		| EEPROM_COMMAND_MESSAGE_DATA_COUNT (count);

	struct can_frame frame =
	{
		.can_id		= canId,
		.can_dlc	= 8,
		.data		=
		{
			instruction,
			instruction >> 8,
			address,
			address >> 8
		}
	};

	return frame;
}

int readMessageParse (uint16_t canId, struct can_frame* frame, uint16_t address, uint8_t count, void* buffer)
{
	uint16_t instruction = frame->data [0] | (frame->data [1] << 8);

	// Check message ID is correct
	if (frame->can_id != (uint16_t) (canId + 1))
	{
		errno = ERRNO_CAN_EEPROM_BAD_RESPONSE_ID;
		return errno;
	}

	// Check message is a read response
	if (!EEPROM_RESPONSE_MESSAGE_READ_NOT_WRITE (instruction))
	{
		errno = ERRNO_CAN_EEPROM_MALFORMED_RESPONSE;
		return errno;
	}

	// Check message is a data response
	if (!EEPROM_RESPONSE_MESSAGE_DATA_NOT_VALIDATION (instruction))
	{
		errno = ERRNO_CAN_EEPROM_MALFORMED_RESPONSE;
		return errno;
	}

	uint16_t frameAddress = frame->data [2] | (frame->data [3] << 8);

	// Check message is the correct address
	if (frameAddress != address)
	{
		errno = ERRNO_CAN_EEPROM_MALFORMED_RESPONSE;
		return errno;
	}

	// Check data count is correct
	uint8_t frameCount = EEPROM_RESPONSE_MESSAGE_DATA_COUNT (instruction);
	if (frameCount != count)
	{
		errno = ERRNO_CAN_EEPROM_MALFORMED_RESPONSE;
		return errno;
	}

	// Success, copy the data into the buffer
	memcpy (buffer, frame->data + 4, count);
	return 0;
}

int writeSingle (uint16_t canId, canSocket_t* socket, uint16_t address, uint8_t count, void* buffer)
{
	struct can_frame commandFrame = writeMessageEncode (canId, address, count, buffer);

	uint8_t bufferRead [4];

	for (uint8_t attempt = 0; attempt < RESPONSE_ATTEMPT_COUNT; ++attempt)
	{
		canSocketTransmit (socket, &commandFrame);
		if (readSingle (canId, socket, address, count, bufferRead) != 0)
			return errno;

		if (memcmp (buffer, bufferRead, count) == 0)
			return 0;
	}

	errno = ERRNO_CAN_EEPROM_WRITE_TIMEOUT;
	return errno;
}

int readSingle (uint16_t canId, canSocket_t* socket, uint16_t address, uint8_t count, void* buffer)
{
	struct can_frame commandFrame = readMessageEncode (canId, address, count);

	for (uint8_t attempt = 0; attempt < RESPONSE_ATTEMPT_COUNT; ++attempt)
	{
		if (canSocketTransmit (socket, &commandFrame) != 0)
			return errno;

		struct timeval deadline;
		struct timeval timeCurrent;
		gettimeofday (&timeCurrent, NULL);
		timeradd (&timeCurrent, &RESPONSE_ATTEMPT_TIMEOUT, &deadline);

		while (timercmp (&timeCurrent, &deadline, <))
		{
			gettimeofday (&timeCurrent, NULL);

			struct can_frame response;
			if (canSocketReceive (socket, &response) != 0)
				continue;

			if (readMessageParse (canId, &response, address, count, buffer) == 0)
				return 0;
		}
	}

	errno = ERRNO_CAN_EEPROM_READ_TIMEOUT;
	return errno;
}

struct can_frame validateMessageEncode (uint16_t canId, bool isValid)
{
	uint16_t instruction = EEPROM_COMMAND_MESSAGE_READ_NOT_WRITE (false)
		| EEPROM_COMMAND_MESSAGE_DATA_NOT_VALIDATION (false)
		| EEPROM_COMMAND_MESSAGE_IS_VALID (isValid);

	struct can_frame frame =
	{
		.can_id		= canId,
		.can_dlc	= 8,
		.data		=
		{
			instruction,
			instruction >> 8
		}
	};

	return frame;
}

struct can_frame isValidMessageEncode (uint16_t canId)
{
	uint16_t instruction = EEPROM_COMMAND_MESSAGE_READ_NOT_WRITE (true)
		| EEPROM_COMMAND_MESSAGE_DATA_NOT_VALIDATION (false);

	struct can_frame frame =
	{
		.can_id		= canId,
		.can_dlc	= 8,
		.data		=
		{
			instruction,
			instruction >> 8,
		}
	};

	return frame;
}

int isValidMessageParse (uint16_t canId, struct can_frame* frame, bool* isValid)
{
	uint16_t instruction = frame->data [0] | (frame->data [1] << 8);

	// Check message ID is correct
	if (frame->can_id != (uint16_t) (canId + 1))
	{
		errno = ERRNO_CAN_EEPROM_BAD_RESPONSE_ID;
		return errno;
	}

	// Check message is a read response
	if (!EEPROM_RESPONSE_MESSAGE_READ_NOT_WRITE (instruction))
	{
		errno = ERRNO_CAN_EEPROM_MALFORMED_RESPONSE;
		return errno;
	}

	// Check message is a validation response
	if (EEPROM_RESPONSE_MESSAGE_DATA_NOT_VALIDATION (instruction))
	{
		errno = ERRNO_CAN_EEPROM_MALFORMED_RESPONSE;
		return errno;
	}

	// Success, read the isValid flag
	*isValid = EEPROM_RESPONSE_MESSAGE_IS_VALID (instruction);
	return 0;
}
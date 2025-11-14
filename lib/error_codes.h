#ifndef ERROR_CODES_H
#define ERROR_CODES_H

// Error Values ---------------------------------------------------------------------------------------------------------------

// Note these must be mutally exclusive with errno values, and therefore start with an absurd offset (1024). While there is no
// guarantee errno doesn't use these values, it is incredibly unlikely.

// Includes -------------------------------------------------------------------------------------------------------------------

// C Standard Library
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

// General Errors -------------------------------------------------------------------------------------------------------------

#define ERRNO_UNKNOWN							1024
#define ERRNO_OS_NOT_SUPPORTED					1025
#define ERRNO_END_OF_FILE						1026

#define ERRMSG_UNKNOWN							"An unknown error occurred"
#define ERRMSG_OS_NOT_SUPPORTED					"The attempted operation is not supported on this operating system"
#define ERRMSG_END_OF_FILE						"Unexpected end of file"

// can_device Module ----------------------------------------------------------------------------------------------------------

#define ERRNO_CAN_DEVICE_UNKNOWN_NAME			1030
#define ERRNO_CAN_DEVICE_BAD_TIMEOUT			1031
#define ERRNO_CAN_DEVICE_TIMEOUT				1032
#define ERRNO_CAN_DEVICE_MISSING_DEVICE			1033

#define ERRMSG_CAN_DEVICE_UNKNOWN_NAME			"The device name does not belong to any known CAN device"
#define ERRMSG_CAN_DEVICE_BAD_TIMEOUT			"The specified timeout is not possible"
#define ERRMSG_CAN_DEVICE_TIMEOUT				"The operation has timed out"
#define ERRMSG_CAN_DEVICE_MISSING_DEVICE		"No CAN device was detected"

// CAN bus errors
#define ERRNO_CAN_DEVICE_BIT_ERROR				1036
#define ERRNO_CAN_DEVICE_BIT_STUFF_ERROR		1037
#define ERRNO_CAN_DEVICE_FORM_ERROR				1038
#define ERRNO_CAN_DEVICE_ACK_ERROR				1039
#define ERRNO_CAN_DEVICE_CRC_ERROR				1040
#define ERRNO_CAN_DEVICE_BUS_OFF				1041
#define ERRNO_CAN_DEVICE_UNSPEC_ERROR			1042

#define ERRMSG_CAN_DEVICE_BIT_ERROR				"A bit transmission error was detected on the CAN bus"
#define ERRMSG_CAN_DEVICE_BIT_STUFF_ERROR		"A bit stuffing error was detected on the CAN bus"
#define ERRMSG_CAN_DEVICE_FORM_ERROR			"A form error was detected on the CAN bus"
#define ERRMSG_CAN_DEVICE_ACK_ERROR				"An ACK error was detected on the CAN bus"
#define ERRMSG_CAN_DEVICE_CRC_ERROR				"A CRC error was detected on the CAN bus"
#define ERRMSG_CAN_DEVICE_BUS_OFF				"The device has entered the CAN bus-off state"
#define ERRMSG_CAN_DEVICE_UNSPEC_ERROR			"Unspecified CAN device error"

// can_database Module --------------------------------------------------------------------------------------------------------

#define ERRNO_CAN_DBC_MESSAGE_COUNT				1048
#define ERRNO_CAN_DBC_SIGNAL_COUNT				1049
#define ERRNO_CAN_DBC_MESSAGE_MISSING			1050
#define ERRNO_CAN_DATABASE_SIGNAL_MISSING		1051
#define ERRNO_CAN_DATABASE_MESSAGE_MISSING		1052

#define ERRMSG_CAN_DBC_MESSAGE_COUNT			"The DBC file exceeds the maximum number of CAN messages"
#define ERRMSG_CAN_DBC_SIGNAL_COUNT				"The DBC file exceeds the maximum number of CAN signals"
#define ERRMSG_CAN_DBC_MESSAGE_MISSING			"The DBC file contains a signal before the first message"
#define ERRMSG_CAN_DATABASE_SIGNAL_MISSING		"No such signal in database"
#define ERRMSG_CAN_DATABASE_MESSAGE_MISSING		"No such message in database"

// cjson Module ---------------------------------------------------------------------------------------------------------------

#define ERRNO_CJSON_EOF							1280
#define ERRNO_CJSON_PARSE_FAIL					1281
#define ERRNO_CJSON_MISSING_KEY					1282
#define ERRNO_CJSON_MAX_SIZE					1283

#define ERRMSG_CJSON_EOF						"Unexpected end of JSON data"
#define ERRMSG_CJSON_PARSE_FAIL					"Invalid JSON data"
#define ERRMSG_CJSON_MISSING_KEY				"Missing JSON key"
#define ERRMSG_CJSON_MAX_SIZE					"The JSON file exceeds the maximum size"

// can_eeprom Module ----------------------------------------------------------------------------------------------------------

#define ERRNO_CAN_EEPROM_INVALID_TYPE			1536
#define ERRNO_CAN_EEPROM_INVALID_MODE			1537
#define ERRNO_CAN_EEPROM_READ_TIMEOUT			1538
#define ERRNO_CAN_EEPROM_WRITE_TIMEOUT			1539
#define ERRNO_CAN_EEPROM_MALFORMED_RESPONSE		1540
#define ERRNO_CAN_EEPROM_BAD_RESPONSE_ID		1541
#define ERRNO_CAN_EEPROM_BAD_KEY				1542
#define ERRNO_CAN_EEPROM_BAD_VALUE				1543
#define ERRNO_CAN_EEPROM_BAD_DIMENSION			1544
#define ERRNO_CAN_EEPROM_READ_ONLY				1545
#define ERRNO_CAN_EEPROM_WRITE_ONLY				1546

#define ERRMSG_CAN_EEPROM_INVALID_TYPE			"Invalid EEPROM variable type"
#define ERRMSG_CAN_EEPROM_INVALID_MODE			"Invalid EEPROM variable mode"
#define ERRMSG_CAN_EEPROM_READ_TIMEOUT			"EEPROM read operation timed out"
#define ERRMSG_CAN_EEPROM_WRITE_TIMEOUT			"EEPROM write operation timed out"
#define ERRMSG_CAN_EEPROM_MALFORMED_RESPONSE	"Received malformed EEPROM response"
#define ERRMSG_CAN_EEPROM_BAD_RESPONSE_ID		"Received an EEPROM response with the incorrect message ID"
#define ERRMSG_CAN_EEPROM_BAD_KEY				"EEPROM data JSON contains an invalid key"
#define ERRMSG_CAN_EEPROM_BAD_VALUE				"EEPROM data JSON contains an invalid value"
#define ERRMSG_CAN_EEPROM_BAD_DIMENSION			"EEPROM data JSON contains a matrix of the incorrect dimension"
#define ERRMSG_CAN_EEPROM_READ_ONLY				"Attempting to write to a read-only variable"
#define ERRMSG_CAN_EEPROM_WRITE_ONLY			"Attempting to read from a write-only variable"

// serial_can Module ----------------------------------------------------------------------------------------------------------
// - These error codes are all defined by the SerialCAN library, their values cannot be changed. Note that SerialCAN defines
//   these as negative, however the can_device wrapper offsets these by 10000 to make the errors line up with standard errno
//   values.

#define ERRNO_SLCAN_EWRN						9998
#define ERRNO_SLCAN_OFFLINE						9991
#define ERRNO_SLCAN_ONLINE						9992
#define ERRNO_SLCAN_MSG_LST						9990
#define ERRNO_SLCAN_RESERVED1					9981
#define ERRNO_SLCAN_TX_BUSY						9980
#define ERRNO_SLCAN_RESERVED2					9979
#define ERRNO_SLCAN_RX_EMPTY					9970
#define ERRNO_SLCAN_QUE_OVR						9960
#define ERRNO_SLCAN_RESERVED3					9959
#define ERRNO_SLCAN_RESOURCE					9910
#define ERRNO_SLCAN_BAUDRATE					9909
#define ERRNO_SLCAN_HANDLE						9908
#define ERRNO_SLCAN_ILLPARA						9907
#define ERRNO_SLCAN_NULLPTR						9906
#define ERRNO_SLCAN_NOTINIT						9905
#define ERRNO_SLCAN_YETINIT						9904
#define ERRNO_SLCAN_LIBRARY						9903
#define ERRNO_SLCAN_NOTSUPP						9902
#define ERRNO_SLCAN_FATAL						9901
#define ERRNO_SLCAN_VENDOR						9900

#define ERRMSG_SLCAN_EWRN						"CAN controller has reached the error warning level"
#define ERRMSG_SLCAN_OFFLINE					"The CAN device is offline"
#define ERRMSG_SLCAN_ONLINE						"The CAN device is already online"
#define ERRMSG_SLCAN_MSG_LST					"A CAN message was lost"
#define ERRMSG_SLCAN_RESERVED1					"Reserved macCAN error"
#define ERRMSG_SLCAN_TX_BUSY					"A transmisstion is already in progress"
#define ERRMSG_SLCAN_RESERVED2					"Reserved macCAN error"
#define ERRMSG_SLCAN_RX_EMPTY					"The receiver queue is empty"
#define ERRMSG_SLCAN_QUE_OVR					"The receiver queue has overrun"
#define ERRMSG_SLCAN_RESERVED3					"Reserved macCAN error"
#define ERRMSG_SLCAN_RESOURCE					"Resource allocation error"
#define ERRMSG_SLCAN_BAUDRATE					"Illegal baudrate"
#define ERRMSG_SLCAN_HANDLE						"Illegal handle"
#define ERRMSG_SLCAN_ILLPARA					"Illegal parameter"
#define ERRMSG_SLCAN_NULLPTR					"Null-pointer assignment"
#define ERRMSG_SLCAN_NOTINIT					"Not initialized"
#define ERRMSG_SLCAN_YETINIT					"Already initialized"
#define ERRMSG_SLCAN_LIBRARY					"Illegal library"
#define ERRMSG_SLCAN_NOTSUPP					"Not supported"
#define ERRMSG_SLCAN_FATAL						"Other error"
#define ERRMSG_SLCAN_VENDOR						"Vendor specific error"

// Functions ------------------------------------------------------------------------------------------------------------------

#define ERRNO_CASE(name) \
	case ERRNO_ ## name: \
		return ERRMSG_ ## name

static inline const char* errorMessage (int errorCode)
{
	switch (errorCode)
	{
	// General Errors
	ERRNO_CASE (UNKNOWN);
	ERRNO_CASE (OS_NOT_SUPPORTED);
	ERRNO_CASE (END_OF_FILE);

	// can_device module
	ERRNO_CASE (CAN_DEVICE_UNKNOWN_NAME);
	ERRNO_CASE (CAN_DEVICE_BAD_TIMEOUT);
	ERRNO_CASE (CAN_DEVICE_TIMEOUT);
	ERRNO_CASE (CAN_DEVICE_MISSING_DEVICE);

	ERRNO_CASE (CAN_DEVICE_BIT_ERROR);
	ERRNO_CASE (CAN_DEVICE_BIT_STUFF_ERROR);
	ERRNO_CASE (CAN_DEVICE_FORM_ERROR);
	ERRNO_CASE (CAN_DEVICE_ACK_ERROR);
	ERRNO_CASE (CAN_DEVICE_CRC_ERROR);
	ERRNO_CASE (CAN_DEVICE_BUS_OFF);
	ERRNO_CASE (CAN_DEVICE_UNSPEC_ERROR);

	// can_database module
	ERRNO_CASE (CAN_DBC_MESSAGE_COUNT);
	ERRNO_CASE (CAN_DBC_SIGNAL_COUNT);
	ERRNO_CASE (CAN_DBC_MESSAGE_MISSING);
	ERRNO_CASE (CAN_DATABASE_SIGNAL_MISSING);
	ERRNO_CASE (CAN_DATABASE_MESSAGE_MISSING);

	// cjson module
	ERRNO_CASE (CJSON_EOF);
	ERRNO_CASE (CJSON_PARSE_FAIL);
	ERRNO_CASE (CJSON_MISSING_KEY);
	ERRNO_CASE (CJSON_MAX_SIZE);

	// can_eeprom module
	ERRNO_CASE (CAN_EEPROM_INVALID_TYPE);
	ERRNO_CASE (CAN_EEPROM_INVALID_MODE);
	ERRNO_CASE (CAN_EEPROM_READ_TIMEOUT);
	ERRNO_CASE (CAN_EEPROM_WRITE_TIMEOUT);
	ERRNO_CASE (CAN_EEPROM_MALFORMED_RESPONSE);
	ERRNO_CASE (CAN_EEPROM_BAD_RESPONSE_ID);
	ERRNO_CASE (CAN_EEPROM_BAD_KEY);
	ERRNO_CASE (CAN_EEPROM_BAD_VALUE);
	ERRNO_CASE (CAN_EEPROM_BAD_DIMENSION);
	ERRNO_CASE (CAN_EEPROM_READ_ONLY);
	ERRNO_CASE (CAN_EEPROM_WRITE_ONLY);

	// serial_can module
	ERRNO_CASE (SLCAN_EWRN);
	ERRNO_CASE (SLCAN_OFFLINE);
	ERRNO_CASE (SLCAN_ONLINE);
	ERRNO_CASE (SLCAN_MSG_LST);
	ERRNO_CASE (SLCAN_RESERVED1);
	ERRNO_CASE (SLCAN_TX_BUSY);
	ERRNO_CASE (SLCAN_RESERVED2);
	ERRNO_CASE (SLCAN_RX_EMPTY);
	ERRNO_CASE (SLCAN_QUE_OVR);
	ERRNO_CASE (SLCAN_RESERVED3);
	ERRNO_CASE (SLCAN_RESOURCE);
	ERRNO_CASE (SLCAN_BAUDRATE);
	ERRNO_CASE (SLCAN_HANDLE);
	ERRNO_CASE (SLCAN_ILLPARA);
	ERRNO_CASE (SLCAN_NULLPTR);
	ERRNO_CASE (SLCAN_NOTINIT);
	ERRNO_CASE (SLCAN_YETINIT);
	ERRNO_CASE (SLCAN_LIBRARY);
	ERRNO_CASE (SLCAN_NOTSUPP);
	ERRNO_CASE (SLCAN_FATAL);
	ERRNO_CASE (SLCAN_VENDOR);

	default:
	 	return strerror (errorCode);
	}
}

/**
 * @brief Prints an error message to @c stderr . The resulting message takes the following format:
 * "<User Message>: <Error Message>"
 *
 * For example:
 * "Failed to open file 'test.txt': No such file or directory."
 *
 * @param message The user message to preface the error message with. Note this can be a format string, in which case the
 * following arguments should be the values to be inserted in place of the format specifiers.
 * @param ... The variadic arguments to insert into the format string. Same convention as the @c printf family of functions.
 * @return The error code associated with the error, that is, the value of @c errno upon entry to the function. Note this
 * resets @c errno , so this return code must be used to determine what the error was.
 */
static inline int errorPrintf (const char* message, ...)
{
	// Store the error that caused the issue and reset errno for later usage.
	int code = errno;
	errno = 0;

	// Print the user message, along with variadic arguments.
	va_list args;
    va_start(args, message);
    vfprintf(stderr, message, args);
    va_end(args);

	// Print the error message.
	fprintf (stderr, ": %s.\n", errorMessage (code));

	// Return the errno value.
	return code;
}

#endif // ERROR_CODES_H
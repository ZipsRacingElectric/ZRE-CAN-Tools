#ifndef ERROR_CODES_H
#define ERROR_CODES_H

// Error Codes ----------------------------------------------------------------------------------------------------------------

// Author: Cole Barach
// Date Created: 2025.02.04

// Description: Common error codes used by all libraries in this project. These error codes are written into errno upon an
//   error.

// Note these must be mutally exclusive with errno values, and therefore start with an absurd offset (1024). While there is no
// guarantee errno doesn't use these values, it is incredibly unlikely.

// Includes -------------------------------------------------------------------------------------------------------------------

// C Standard Library
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

#define ERROR_CODE_TO_MESSAGE_CASE(name) \
	case ERRNO_ ## name: \
		return ERRMSG_ ## name

/**
 * @brief Converts an error code into a message describing the error. This function is comparable to @c strerror , however the
 * key difference is this function support the custom error codes defined in this file.
 * @note This function is always preferrable to using @c strerror .
 * @param code The error code, as returned by a function or retrieved from @c errno .
 * @return A user-friendly string describing the error.
 */
static inline const char* errorCodeToMessage (int code)
{
	switch (code)
	{
	// General Errors
	ERROR_CODE_TO_MESSAGE_CASE (UNKNOWN);
	ERROR_CODE_TO_MESSAGE_CASE (OS_NOT_SUPPORTED);
	ERROR_CODE_TO_MESSAGE_CASE (END_OF_FILE);

	// can_device module
	ERROR_CODE_TO_MESSAGE_CASE (CAN_DEVICE_UNKNOWN_NAME);
	ERROR_CODE_TO_MESSAGE_CASE (CAN_DEVICE_BAD_TIMEOUT);
	ERROR_CODE_TO_MESSAGE_CASE (CAN_DEVICE_TIMEOUT);
	ERROR_CODE_TO_MESSAGE_CASE (CAN_DEVICE_MISSING_DEVICE);

	ERROR_CODE_TO_MESSAGE_CASE (CAN_DEVICE_BIT_ERROR);
	ERROR_CODE_TO_MESSAGE_CASE (CAN_DEVICE_BIT_STUFF_ERROR);
	ERROR_CODE_TO_MESSAGE_CASE (CAN_DEVICE_FORM_ERROR);
	ERROR_CODE_TO_MESSAGE_CASE (CAN_DEVICE_ACK_ERROR);
	ERROR_CODE_TO_MESSAGE_CASE (CAN_DEVICE_CRC_ERROR);
	ERROR_CODE_TO_MESSAGE_CASE (CAN_DEVICE_BUS_OFF);
	ERROR_CODE_TO_MESSAGE_CASE (CAN_DEVICE_UNSPEC_ERROR);

	// can_database module
	ERROR_CODE_TO_MESSAGE_CASE (CAN_DBC_MESSAGE_COUNT);
	ERROR_CODE_TO_MESSAGE_CASE (CAN_DBC_SIGNAL_COUNT);
	ERROR_CODE_TO_MESSAGE_CASE (CAN_DBC_MESSAGE_MISSING);
	ERROR_CODE_TO_MESSAGE_CASE (CAN_DATABASE_SIGNAL_MISSING);
	ERROR_CODE_TO_MESSAGE_CASE (CAN_DATABASE_MESSAGE_MISSING);

	// cjson module
	ERROR_CODE_TO_MESSAGE_CASE (CJSON_EOF);
	ERROR_CODE_TO_MESSAGE_CASE (CJSON_PARSE_FAIL);
	ERROR_CODE_TO_MESSAGE_CASE (CJSON_MISSING_KEY);
	ERROR_CODE_TO_MESSAGE_CASE (CJSON_MAX_SIZE);

	// can_eeprom module
	ERROR_CODE_TO_MESSAGE_CASE (CAN_EEPROM_INVALID_TYPE);
	ERROR_CODE_TO_MESSAGE_CASE (CAN_EEPROM_INVALID_MODE);
	ERROR_CODE_TO_MESSAGE_CASE (CAN_EEPROM_READ_TIMEOUT);
	ERROR_CODE_TO_MESSAGE_CASE (CAN_EEPROM_WRITE_TIMEOUT);
	ERROR_CODE_TO_MESSAGE_CASE (CAN_EEPROM_MALFORMED_RESPONSE);
	ERROR_CODE_TO_MESSAGE_CASE (CAN_EEPROM_BAD_RESPONSE_ID);
	ERROR_CODE_TO_MESSAGE_CASE (CAN_EEPROM_BAD_KEY);
	ERROR_CODE_TO_MESSAGE_CASE (CAN_EEPROM_BAD_VALUE);
	ERROR_CODE_TO_MESSAGE_CASE (CAN_EEPROM_BAD_DIMENSION);
	ERROR_CODE_TO_MESSAGE_CASE (CAN_EEPROM_READ_ONLY);
	ERROR_CODE_TO_MESSAGE_CASE (CAN_EEPROM_WRITE_ONLY);

	// serial_can module
	ERROR_CODE_TO_MESSAGE_CASE (SLCAN_EWRN);
	ERROR_CODE_TO_MESSAGE_CASE (SLCAN_OFFLINE);
	ERROR_CODE_TO_MESSAGE_CASE (SLCAN_ONLINE);
	ERROR_CODE_TO_MESSAGE_CASE (SLCAN_MSG_LST);
	ERROR_CODE_TO_MESSAGE_CASE (SLCAN_RESERVED1);
	ERROR_CODE_TO_MESSAGE_CASE (SLCAN_TX_BUSY);
	ERROR_CODE_TO_MESSAGE_CASE (SLCAN_RESERVED2);
	ERROR_CODE_TO_MESSAGE_CASE (SLCAN_RX_EMPTY);
	ERROR_CODE_TO_MESSAGE_CASE (SLCAN_QUE_OVR);
	ERROR_CODE_TO_MESSAGE_CASE (SLCAN_RESERVED3);
	ERROR_CODE_TO_MESSAGE_CASE (SLCAN_RESOURCE);
	ERROR_CODE_TO_MESSAGE_CASE (SLCAN_BAUDRATE);
	ERROR_CODE_TO_MESSAGE_CASE (SLCAN_HANDLE);
	ERROR_CODE_TO_MESSAGE_CASE (SLCAN_ILLPARA);
	ERROR_CODE_TO_MESSAGE_CASE (SLCAN_NULLPTR);
	ERROR_CODE_TO_MESSAGE_CASE (SLCAN_NOTINIT);
	ERROR_CODE_TO_MESSAGE_CASE (SLCAN_YETINIT);
	ERROR_CODE_TO_MESSAGE_CASE (SLCAN_LIBRARY);
	ERROR_CODE_TO_MESSAGE_CASE (SLCAN_NOTSUPP);
	ERROR_CODE_TO_MESSAGE_CASE (SLCAN_FATAL);
	ERROR_CODE_TO_MESSAGE_CASE (SLCAN_VENDOR);

	// For all other errors, default to the C standard library
	default:
	 	return strerror (code);
	}
}

#endif // ERROR_CODES_H
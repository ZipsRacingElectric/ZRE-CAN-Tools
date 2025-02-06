#ifndef ERROR_CODES_H
#define ERROR_CODES_H

// Includes -------------------------------------------------------------------------------------------------------------------

// C Standard Library
#include <string.h>

// Error Values ---------------------------------------------------------------------------------------------------------------

// Note these must be mutally exclusive with errno values, and therefore start with an absurd offset (1024). While there is no
// guarantee errno doesn't use these values, it is incredibly unlikely.

#define ERRNO_CAN_DBC_MESSAGE_COUNT				1024
#define ERRNO_CAN_DBC_SIGNAL_COUNT				1025
#define ERRNO_CAN_DBC_MESSAGE_MISSING			1026

#define ERRNO_CJSON_EOF							1280
#define ERRNO_CJSON_PARSE_FAIL					1281
#define ERRNO_CJSON_MISSING_KEY					1282

#define ERRNO_CAN_EEPROM_INVALID_TYPE			1536
#define ERRNO_CAN_EEPROM_READ_TIMEOUT			1537
#define ERRNO_CAN_EEPROM_WRITE_TIMEOUT			1538
#define ERRNO_CAN_EEPROM_VALIDATE_TIMEOUT		1539
#define ERRNO_CAN_EEPROM_IS_VALID_TIMEOUT		1540
#define ERRNO_CAN_EEPROM_MALFORMED_RESPONSE		1541
#define ERRNO_CAN_EEPROM_BAD_RESPONSE_ID		1542
#define ERRNO_CAN_EEPROM_BAD_KEY				1543
#define ERRNO_CAN_EEPROM_BAD_VALUE				1544
#define ERRNO_CAN_EEPROM_BAD_DIMENSION			1545

// Error Messages -------------------------------------------------------------------------------------------------------------

#define ERRMSG_CAN_DBC_MESSAGE_COUNT			"The DBC file exceeds the maximum number of CAN messages"
#define ERRMSG_CAN_DBC_SIGNAL_COUNT				"The DBC file exceeds the maximum number of CAN signals"
#define ERRMSG_CAN_DBC_MESSAGE_MISSING			"The DBC file contains a signal before the first message"

#define ERRMSG_CJSON_EOF						"Unexpected end of JSON data"
#define ERRMSG_CJSON_PARSE_FAIL					"Invalid JSON data"
#define ERRMSG_CJSON_MISSING_KEY				"Missing JSON key"

#define ERRMSG_CAN_EEPROM_INVALID_TYPE			"Invalid EEPROM variable type"
#define ERRMSG_CAN_EEPROM_READ_TIMEOUT			"EEPROM read operation timed out"
#define ERRMSG_CAN_EEPROM_WRITE_TIMEOUT			"EEPROM write operation timed out"
#define ERRMSG_CAN_EEPROM_VALIDATE_TIMEOUT		"EEPROM validation operation timed out"
#define ERRMSG_CAN_EEPROM_IS_VALID_TIMEOUT		"EEPROM validity request timed out"
#define ERRMSG_CAN_EEPROM_MALFORMED_RESPONSE	"Received malformed EEPROM response"
#define ERRMSG_CAN_EEPROM_BAD_RESPONSE_ID		"Received an EEPROM response with the incorrect message ID"
#define ERRMSG_CAN_EEPROM_BAD_KEY				"EEPROM data JSON contains an invalid key"
#define ERRMSG_CAN_EEPROM_BAD_VALUE				"EEPROM data JSON contains an invalid value"
#define ERRMSG_CAN_EEPROM_BAD_DIMENSION			"EEPROM data JSON contains a matrix of the incorrect dimension"

// Functions ------------------------------------------------------------------------------------------------------------------

static inline const char* errorMessage (int errorCode)
{
	if (errorCode == ERRNO_CAN_DBC_MESSAGE_COUNT)
		return ERRMSG_CAN_DBC_MESSAGE_COUNT;
	else if (errorCode == ERRNO_CAN_DBC_SIGNAL_COUNT)
		return ERRMSG_CAN_DBC_SIGNAL_COUNT;
	else if (errorCode == ERRNO_CAN_DBC_MESSAGE_MISSING)
		return ERRMSG_CAN_DBC_MESSAGE_MISSING;
	else if (errorCode == ERRNO_CJSON_EOF)
		return ERRMSG_CJSON_EOF;
	else if (errorCode == ERRNO_CJSON_PARSE_FAIL)
		return ERRMSG_CJSON_PARSE_FAIL;
	else if (errorCode == ERRNO_CJSON_MISSING_KEY)
		return ERRMSG_CJSON_MISSING_KEY;
	else if (errorCode == ERRNO_CAN_EEPROM_INVALID_TYPE)
		return ERRMSG_CAN_EEPROM_INVALID_TYPE;
	else if (errorCode == ERRNO_CAN_EEPROM_READ_TIMEOUT)
		return ERRMSG_CAN_EEPROM_READ_TIMEOUT;
	else if (errorCode == ERRNO_CAN_EEPROM_WRITE_TIMEOUT)
		return ERRMSG_CAN_EEPROM_WRITE_TIMEOUT;
	else if (errorCode == ERRNO_CAN_EEPROM_VALIDATE_TIMEOUT)
		return ERRMSG_CAN_EEPROM_VALIDATE_TIMEOUT;
	else if (errorCode == ERRNO_CAN_EEPROM_IS_VALID_TIMEOUT)
		return ERRMSG_CAN_EEPROM_IS_VALID_TIMEOUT;
	else if (errorCode == ERRNO_CAN_EEPROM_MALFORMED_RESPONSE)
		return ERRMSG_CAN_EEPROM_MALFORMED_RESPONSE;
	else if (errorCode == ERRNO_CAN_EEPROM_BAD_RESPONSE_ID)
		return ERRMSG_CAN_EEPROM_BAD_RESPONSE_ID;
	else if (errorCode == ERRNO_CAN_EEPROM_BAD_KEY)
		return ERRMSG_CAN_EEPROM_BAD_KEY;
	else if (errorCode == ERRNO_CAN_EEPROM_BAD_VALUE)
		return ERRMSG_CAN_EEPROM_BAD_VALUE;
	else if (errorCode == ERRNO_CAN_EEPROM_BAD_DIMENSION)
		return ERRMSG_CAN_EEPROM_BAD_DIMENSION;
	else
	 	return strerror (errorCode);
}

#endif // ERROR_CODES_H
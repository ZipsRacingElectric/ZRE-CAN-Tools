// Header
#include "error_codes.h"

// C Standard Library
#include <string.h>

const char* errorMessage (int errorCode)
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
	else
	 	return strerror (errorCode);
}
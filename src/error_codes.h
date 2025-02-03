#ifndef ERROR_CODES_H
#define ERROR_CODES_H

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

const char* errorMessage (int errorCode);

#endif // ERROR_CODES_H
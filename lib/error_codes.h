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

#define ERRNO_OS_NOT_SUPPORTED					1030
#define ERRNO_CAN_DEVICE_UNKNOWN_NAME			1031
#define ERRNO_CAN_DEVICE_BAD_TIMEOUT			1032

#define ERRNO_CAN_DATABASE_SIGNAL_MISSING		1048

#define ERRNO_CJSON_EOF							1280
#define ERRNO_CJSON_PARSE_FAIL					1281
#define ERRNO_CJSON_MISSING_KEY					1282
#define ERRNO_CJSON_MAX_SIZE					1283

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

#define ERRNO_SLCAN_BOFF						9999
#define ERRNO_SLCAN_EWRN						9998
#define ERRNO_SLCAN_BERR						9997
#define ERRNO_SLCAN_OFFLINE						9991
#define ERRNO_SLCAN_ONLINE						9992
#define ERRNO_SLCAN_MSG_LST						9990
#define ERRNO_SLCAN_LEC_STUFF					9989
#define ERRNO_SLCAN_LEC_FORM					9988
#define ERRNO_SLCAN_LEC_ACK						9987
#define ERRNO_SLCAN_LEC_BIT1					9986
#define ERRNO_SLCAN_LEC_BIT0					9985
#define ERRNO_SLCAN_LEC_CRC						9984
#define ERRNO_SLCAN_RESERVED1					9981
#define ERRNO_SLCAN_TX_BUSY						9980
#define ERRNO_SLCAN_RESERVED2					9979
#define ERRNO_SLCAN_RX_EMPTY					9970
#define ERRNO_SLCAN_QUE_OVR						9960
#define ERRNO_SLCAN_RESERVED3					9959
#define ERRNO_SLCAN_TIMEOUT						9950
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

// Error Messages -------------------------------------------------------------------------------------------------------------

#define ERRMSG_CAN_DBC_MESSAGE_COUNT			"The DBC file exceeds the maximum number of CAN messages"
#define ERRMSG_CAN_DBC_SIGNAL_COUNT				"The DBC file exceeds the maximum number of CAN signals"
#define ERRMSG_CAN_DBC_MESSAGE_MISSING			"The DBC file contains a signal before the first message"

#define ERRMSG_OS_NOT_SUPPORTED					"The attempted operation is not supported on this operating system"
#define ERRMSG_CAN_DEVICE_UNKNOWN_NAME			"The device name does not belong to any known CAN device"
#define ERRMSG_CAN_DEVICE_BAD_TIMEOUT			"The specified timeout is not possible"

#define ERRMSG_CAN_DATABASE_SIGNAL_MISSING		"No such signal in database"

#define ERRMSG_CJSON_EOF						"Unexpected end of JSON data"
#define ERRMSG_CJSON_PARSE_FAIL					"Invalid JSON data"
#define ERRMSG_CJSON_MISSING_KEY				"Missing JSON key"
#define ERRMSG_CJSON_MAX_SIZE					"The JSON file exceeds the maximum size"

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

#define ERRMSG_SLCAN_BOFF						"CAN busoff status"
#define ERRMSG_SLCAN_EWRN						"CAN error warning status"
#define ERRMSG_SLCAN_BERR						"CAN bus error"
#define ERRMSG_SLCAN_OFFLINE					"CAN device not started"
#define ERRMSG_SLCAN_ONLINE						"CAN device already started"
#define ERRMSG_SLCAN_MSG_LST					"CAN message lost"
#define ERRMSG_SLCAN_LEC_STUFF					"LEC stuff error"
#define ERRMSG_SLCAN_LEC_FORM					"LEC form error"
#define ERRMSG_SLCAN_LEC_ACK					"LEC acknowledge error"
#define ERRMSG_SLCAN_LEC_BIT1					"LEC recessive bit error"
#define ERRMSG_SLCAN_LEC_BIT0					"LEC dominant bit error"
#define ERRMSG_SLCAN_LEC_CRC					"LEC checksum error"
#define ERRMSG_SLCAN_RESERVED1					"RIP error frame"
#define ERRMSG_SLCAN_TX_BUSY					"Transmitter busy"
#define ERRMSG_SLCAN_RESERVED2					"Reserved macCAN error"
#define ERRMSG_SLCAN_RX_EMPTY					"Receiver empty"
#define ERRMSG_SLCAN_QUE_OVR					"Queue overrun"
#define ERRMSG_SLCAN_RESERVED3					"Reserved macCAN error"
#define ERRMSG_SLCAN_TIMEOUT					"A timeout occurred"
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

static inline const char* errorMessage (int errorCode)
{
	switch (errorCode)
	{
	case ERRNO_CAN_DBC_MESSAGE_COUNT:
		return ERRMSG_CAN_DBC_MESSAGE_COUNT;
	case ERRNO_CAN_DBC_SIGNAL_COUNT:
		return ERRMSG_CAN_DBC_SIGNAL_COUNT;
	case ERRNO_CAN_DBC_MESSAGE_MISSING:
		return ERRMSG_CAN_DBC_MESSAGE_MISSING;

	case ERRNO_OS_NOT_SUPPORTED:
		return ERRMSG_OS_NOT_SUPPORTED;
	case ERRNO_CAN_DEVICE_UNKNOWN_NAME:
		return ERRMSG_CAN_DEVICE_UNKNOWN_NAME;
	case ERRNO_CAN_DEVICE_BAD_TIMEOUT:
		return ERRMSG_CAN_DEVICE_BAD_TIMEOUT;

	case ERRNO_CAN_DATABASE_SIGNAL_MISSING:
		return ERRMSG_CAN_DATABASE_SIGNAL_MISSING;

	case ERRNO_CJSON_EOF:
		return ERRMSG_CJSON_EOF;
	case ERRNO_CJSON_PARSE_FAIL:
		return ERRMSG_CJSON_PARSE_FAIL;
	case ERRNO_CJSON_MISSING_KEY:
		return ERRMSG_CJSON_MISSING_KEY;
	case ERRNO_CJSON_MAX_SIZE:
		return ERRMSG_CJSON_MAX_SIZE;

	case ERRNO_CAN_EEPROM_INVALID_TYPE:
		return ERRMSG_CAN_EEPROM_INVALID_TYPE;
	case ERRNO_CAN_EEPROM_INVALID_MODE:
		return ERRMSG_CAN_EEPROM_INVALID_MODE;
	case ERRNO_CAN_EEPROM_READ_TIMEOUT:
		return ERRMSG_CAN_EEPROM_READ_TIMEOUT;
	case ERRNO_CAN_EEPROM_WRITE_TIMEOUT:
		return ERRMSG_CAN_EEPROM_WRITE_TIMEOUT;
	case ERRNO_CAN_EEPROM_MALFORMED_RESPONSE:
		return ERRMSG_CAN_EEPROM_MALFORMED_RESPONSE;
	case ERRNO_CAN_EEPROM_BAD_RESPONSE_ID:
		return ERRMSG_CAN_EEPROM_BAD_RESPONSE_ID;
	case ERRNO_CAN_EEPROM_BAD_KEY:
		return ERRMSG_CAN_EEPROM_BAD_KEY;
	case ERRNO_CAN_EEPROM_BAD_VALUE:
		return ERRMSG_CAN_EEPROM_BAD_VALUE;
	case ERRNO_CAN_EEPROM_BAD_DIMENSION:
		return ERRMSG_CAN_EEPROM_BAD_DIMENSION;
	case ERRNO_CAN_EEPROM_READ_ONLY:
		return ERRMSG_CAN_EEPROM_READ_ONLY;
	case ERRNO_CAN_EEPROM_WRITE_ONLY:
		return ERRMSG_CAN_EEPROM_WRITE_ONLY;

	case ERRNO_SLCAN_BOFF:
		return ERRMSG_SLCAN_BOFF;
	case ERRNO_SLCAN_EWRN:
		return ERRMSG_SLCAN_EWRN;
	case ERRNO_SLCAN_BERR:
		return ERRMSG_SLCAN_BERR;
	case ERRNO_SLCAN_OFFLINE:
		return ERRMSG_SLCAN_OFFLINE;
	case ERRNO_SLCAN_ONLINE:
		return ERRMSG_SLCAN_ONLINE;
	case ERRNO_SLCAN_MSG_LST:
		return ERRMSG_SLCAN_MSG_LST;
	case ERRNO_SLCAN_LEC_STUFF:
		return ERRMSG_SLCAN_LEC_STUFF;
	case ERRNO_SLCAN_LEC_FORM:
		return ERRMSG_SLCAN_LEC_FORM;
	case ERRNO_SLCAN_LEC_ACK:
		return ERRMSG_SLCAN_LEC_ACK;
	case ERRNO_SLCAN_LEC_BIT1:
		return ERRMSG_SLCAN_LEC_BIT1;
	case ERRNO_SLCAN_LEC_BIT0:
		return ERRMSG_SLCAN_LEC_BIT0;
	case ERRNO_SLCAN_LEC_CRC:
		return ERRMSG_SLCAN_LEC_CRC;
	case ERRNO_SLCAN_RESERVED1:
		return ERRMSG_SLCAN_RESERVED1;
	case ERRNO_SLCAN_TX_BUSY:
		return ERRMSG_SLCAN_TX_BUSY;
	case ERRNO_SLCAN_RESERVED2:
		return ERRMSG_SLCAN_RESERVED2;
	case ERRNO_SLCAN_RX_EMPTY:
		return ERRMSG_SLCAN_RX_EMPTY;
	case ERRNO_SLCAN_QUE_OVR:
		return ERRMSG_SLCAN_QUE_OVR;
	case ERRNO_SLCAN_RESERVED3:
		return ERRMSG_SLCAN_RESERVED3;
	case ERRNO_SLCAN_TIMEOUT:
		return ERRMSG_SLCAN_TIMEOUT;
	case ERRNO_SLCAN_RESOURCE:
		return ERRMSG_SLCAN_RESOURCE;
	case ERRNO_SLCAN_BAUDRATE:
		return ERRMSG_SLCAN_BAUDRATE;
	case ERRNO_SLCAN_HANDLE:
		return ERRMSG_SLCAN_HANDLE;
	case ERRNO_SLCAN_ILLPARA:
		return ERRMSG_SLCAN_ILLPARA;
	case ERRNO_SLCAN_NULLPTR:
		return ERRMSG_SLCAN_NULLPTR;
	case ERRNO_SLCAN_NOTINIT:
		return ERRMSG_SLCAN_NOTINIT;
	case ERRNO_SLCAN_YETINIT:
		return ERRMSG_SLCAN_YETINIT;
	case ERRNO_SLCAN_LIBRARY:
		return ERRMSG_SLCAN_LIBRARY;
	case ERRNO_SLCAN_NOTSUPP:
		return ERRMSG_SLCAN_NOTSUPP;
	case ERRNO_SLCAN_FATAL:
		return ERRMSG_SLCAN_FATAL;
	case ERRNO_SLCAN_VENDOR:
		return ERRMSG_SLCAN_VENDOR;

	default:
	 	return strerror (errorCode);
	}
}

#endif // ERROR_CODES_H
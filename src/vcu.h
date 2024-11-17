#ifndef VCU_H
#define VCU_H

// Vehicle Control Unit CAN Interface -----------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2024.10.30
//
// Description: Set of functions for interfacing with the VCU via CAN.

// Includes -------------------------------------------------------------------------------------------------------------------

// SocketCAN Libraries
#include <linux/can.h>

// C Standard Library
#include <stdint.h>

// Datatypes ------------------------------------------------------------------------------------------------------------------

typedef enum
{
	VCU_VARIABLE_TYPE_UINT8		= 0,
	VCU_VARIABLE_TYPE_UINT16	= 1,
	VCU_VARIABLE_TYPE_UINT32	= 2,
	VCU_VARIABLE_TYPE_FLOAT		= 3,
} vcuVariableType_t;

typedef struct
{
	uint16_t			address;
	const char*			name;
	vcuVariableType_t	type;
} vcuVariable_t;

// Functions ------------------------------------------------------------------------------------------------------------------

struct can_frame vcuEepromMessageEncode (vcuVariable_t* variable, void* data);

struct can_frame vcuEepromMessagePrompt ();

#endif // VCU_H
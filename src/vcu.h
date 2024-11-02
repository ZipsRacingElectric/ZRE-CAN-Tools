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

enum vcuVariableType
{
	VCU_VARIABLE_TYPE_UINT16	= 0,
	VCU_VARIABLE_TYPE_UINT32	= 1,
	VCU_VARIABLE_TYPE_FLOAT		= 2,
};

typedef enum vcuVariableType vcuVariableType_t;

struct vcuVariable
{
	uint16_t			address;
	const char*			name;
	vcuVariableType_t	type;
};

typedef struct vcuVariable vcuVariable_t;

// Functions ------------------------------------------------------------------------------------------------------------------

struct can_frame vcuEepromMessageEncode (vcuVariable_t* variable, void* data);

struct can_frame vcuEepromMessagePrompt ();

#endif // VCU_H
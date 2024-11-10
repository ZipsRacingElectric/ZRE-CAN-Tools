// Header
#include "vcu.h"

// C Standard Library
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

// Message IDs ----------------------------------------------------------------------------------------------------------------

#define EEPROM_MESSAGE_ID 0x750

// EEPROM Map -----------------------------------------------------------------------------------------------------------------

uint16_t vcuVariableSizes [] =
{
	sizeof (uint16_t),
	sizeof (uint32_t),
	sizeof (float)
};

#define EEPROM_VARIABLE_COUNT (sizeof (eepromMap) / sizeof (vcuVariable_t))
vcuVariable_t eepromMap [] =
{
	{
		.address = 0x10,
		.name = "APPS_1_MIN",
		.type = VCU_VARIABLE_TYPE_UINT16
	},
	{
		.address = 0x12,
		.name = "APPS_1_MAX",
		.type = VCU_VARIABLE_TYPE_UINT16
	},
	{
		.address = 0x14,
		.name = "APPS_2_MIN",
		.type = VCU_VARIABLE_TYPE_UINT16
	},
	{
		.address = 0x16,
		.name = "APPS_2_MAX",
		.type = VCU_VARIABLE_TYPE_UINT16
	},
	{
		.address = 0x18,
		.name = "BSE_F_MIN",
		.type = VCU_VARIABLE_TYPE_UINT16
	},
	{
		.address = 0x1A,
		.name = "BSE_F_MAX",
		.type = VCU_VARIABLE_TYPE_UINT16
	},
	{
		.address = 0x1C,
		.name = "BSE_R_MIN",
		.type = VCU_VARIABLE_TYPE_UINT16
	},
	{
		.address = 0x1E,
		.name = "BSE_R_MAX",
		.type = VCU_VARIABLE_TYPE_UINT16
	},
};

// Functions ------------------------------------------------------------------------------------------------------------------

struct can_frame vcuEepromMessageEncode (vcuVariable_t* variable, void* data)
{
	struct can_frame frame =
	{
		.can_id		= EEPROM_MESSAGE_ID,
		.can_dlc	= 8
	};

	uint16_t size = vcuVariableSizes [variable->type];

	frame.data [0] = variable->address;
	frame.data [1] = variable->address << 8;
	frame.data [2] = size;
	frame.data [3] = size << 8;

	memcpy (frame.data + 4, data, size);

	return frame;
}

struct can_frame vcuEepromMessagePrompt ()
{
	char buffer [256];
	vcuVariable_t* variable = NULL;

	while (true)
	{
		printf ("Enter the key of the variable to send: ");
		fgets (buffer, sizeof (buffer), stdin);
		buffer [strcspn (buffer, "\r\n")] = '\0';

		for (uint16_t index = 0; index < EEPROM_VARIABLE_COUNT; ++index)
		{
			if (strcmp (buffer, eepromMap [index].name) != 0)
				continue;

			variable = eepromMap + index;
			break;
		}

		if (variable != NULL)
			break;

		printf ("Unknown key '%s'\n", buffer);
	}

	uint32_t variableData;

	switch (variable->type)
	{
	case VCU_VARIABLE_TYPE_UINT16:
		printf ("Enter the value of the variable (uint16_t): ");
		fscanf (stdin, "%u%*1[\n]", &variableData);
		break;
	case VCU_VARIABLE_TYPE_UINT32:
		printf ("Enter the value of the variable (uint32_t): ");
		fscanf (stdin, "%u%*1[\n]", &variableData);
		break;
	case VCU_VARIABLE_TYPE_FLOAT:
		printf ("Enter the value of the variable (float): ");
		fscanf (stdin, "%f%*1[\n]", (float*) &variableData);
		break;
	}

	return vcuEepromMessageEncode (variable, &variableData);
}
// Header
#include "can_device_stdio.h"

#include <stdlib.h>

int strToCanId (uint32_t* id, bool* ide, const char* str)
{
	char* end;
	uint32_t idValue = strtoul (str, &end, 0);
	if (end == str || (*end != '\0' && *end != 'x'))
	{
		errno = EINVAL;
		return errno;
	}

	*id = idValue;
	*ide = *end == 'x';

	return 0;
}

int strToCanFrame (canFrame_t* frame, char* str)
{
	char* savePtr;

	// Parse the CAN ID
	char* idStr = strtok_r (str, "[", &savePtr);
	if (idStr == NULL)
	{
		errno = EINVAL;
		return errno;
	}

	if (strToCanId (&frame->id, &frame->ide, idStr) != 0)
	{
		errno = EINVAL;
		return errno;
	}

	// Parse the frame payload
	frame->dlc = 0;
	while (true)
	{
		char* byteStr = strtok_r (NULL, ",]", &savePtr);
		if (byteStr == NULL)
			break;

		if (frame->dlc == 8)
		{
			errno = EINVAL;
			return errno;
		}

		char* end;
		uint8_t byteValue = (uint8_t) strtoul (byteStr, &end, 0);
		if (end == byteStr)
			return EINVAL;

		frame->data [frame->dlc] = byteValue;
		++frame->dlc;
	}

	return 0;
}

int fprintCanId (FILE* stream, uint32_t id, bool ide)
{
	int cumulative = 0;

	int code = fprintf (stream, "0x%03X", id);
	if (code < 0)
		return code;
	cumulative += code;

	if (ide)
	{
		code = fprintf (stream, "x");
		if (code < 0)
			return code;
		cumulative += code;
	}

	return cumulative;
}

int fprintCanFrame (FILE* stream, canFrame_t* frame)
{
	int cumulative = 0;

	int code = fprintCanId (stream, frame->id, frame->ide);
	if (code < 0)
		return code;
	cumulative += code;

	code = fprintf (stream, "[");
	if (code < 0)
		return code;
	cumulative += code;

	for (size_t index = 0; index < frame->dlc; ++index)
	{
		code = fprintf (stream, "0x%02X", frame->data [index]);
		if (code < 0)
			return code;
		cumulative += code;

		if (index != frame->dlc - 1)
		{
			code = fprintf (stream, ",");
			if (code < 0)
				return code;
			cumulative += code;
		}
	}

	code = fprintf (stream, "]");
	if (code < 0)
		return code;
	cumulative += code;

	return cumulative;
}
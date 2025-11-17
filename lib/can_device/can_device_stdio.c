// Header
#include "can_device_stdio.h"

// C Standard Library
#include <errno.h>
#include <stdlib.h>

int strToCanId (uint32_t* id, bool* ide, bool* rtr, const char* str)
{
	// Parse the numeric part of the ID.
	char* end;
	*id = strtoul (str, &end, 0);
	if (end == str)
	{
		errno = EINVAL;
		return errno;
	}

	// Horrible flag validation.
	if (end [0] != '\0')
	{
		if (end [0] != 'r' && end [0] != 'x')
		{
			errno = EINVAL;
			return errno;
		}

		if (end [1] != '\0')
		{
			if ((end [1] != 'x' && end [1] != 'r') || end [2] != '\0')
			{
				errno = EINVAL;
				return errno;
			}
		}
	}

	// Parse the IDE flag
	*ide = end [0] == 'x' || (end [0] != '\0' && end [1] == 'x');

	// Parse the RTR flag
	*rtr = end [0] == 'r' || (end [0] != '\0' && end [1] == 'r');

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

	if (strToCanId (&frame->id, &frame->ide, &frame->rtr, idStr) != 0)
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

int fprintCanId (FILE* stream, uint32_t id, bool ide, bool rtr)
{
	if (ide)
	{
		// Extended ID & RTR
		if (rtr)
			return fprintf (stream, "0x%03Xxr", id);

		// Extended ID
		return fprintf (stream, "0x%03Xx", id);
	}

	// RTR
	if (rtr)
		return fprintf (stream, "0x%03Xr", id);

	// Standard ID
	return fprintf (stream, "0x%03X", id);
}

int fprintCanFrame (FILE* stream, canFrame_t* frame)
{
	int cumulative = 0;

	// Print the CAN ID
	int code = fprintCanId (stream, frame->id, frame->ide, frame->rtr);
	if (code < 0)
		return code;
	cumulative += code;

	// Print the start of the payload
	code = fprintf (stream, "[");
	if (code < 0)
		return code;
	cumulative += code;

	// Print the payload contents
	for (size_t index = 0; index < frame->dlc; ++index)
	{
		code = fprintf (stream, "0x%02X", frame->data [index]);
		if (code < 0)
			return code;
		cumulative += code;

		if (index + 1 != frame->dlc)
		{
			code = fprintf (stream, ",");
			if (code < 0)
				return code;
			cumulative += code;
		}
	}

	// Print the end of the payload
	code = fprintf (stream, "]");
	if (code < 0)
		return code;
	cumulative += code;

	return cumulative;
}

int snprintCanId (char* str, size_t n, uint32_t id, bool ide, bool rtr)
{
	if (ide)
	{
		// Extended ID & RTR
		if (rtr)
			return snprintf (str, n, "0x%03Xxr", id);

		// Extended ID
		return snprintf (str, n, "0x%03Xx", id);
	}

	// RTR
	if (rtr)
		return snprintf (str, n, "0x%03Xr", id);

	// Standard ID
	return snprintf (str, n, "0x%03X", id);
}

int fprintCanDeviceNameHelp (FILE* stream, const char* indent)
{
	return fprintf (stream, ""
		"%s<Device Name>         - The adapter-specific identity of the CAN device.\n"
		"%s    can*@<Baud>       - SocketCAN device, must be already initialized and\n"
		"%s                        setup. Ex. 'can0@1000000'. Note, baudrate is optional\n"
		"%s                        for most applications.\n"
		"%s    vcan*@<Baud>      - Virtual SocketCAN device, must be already initialized\n"
		"%s                        and setup. Note, baudrate is optional for most\n"
		"%s                        applications.\n"
		"%s    <Port>@<Baud>     - SLCAN device, must be a CANable device. CAN baudrate is\n"
		"%s                        initialized to <Baud> bit/s. Ex 'COM3@1000000' for\n"
		"%s                        Windows and '/dev/ttyACM0@1000000' for Linux.\n"
		"\n",
		indent, indent, indent, indent, indent, indent, indent, indent, indent, indent);
}

int fprintCanIdHelp (FILE* stream, const char* indent)
{
	return fprintf (stream, ""
		"%s<CAN ID>              - The identifier of a frame.\n"
		"%s    <SID>             - Standard CAN ID, may be decimal or hexadecimal\n"
		"%s                        (hex should be prefixed with '0x').\n"
		"%s    <SID>r            - Standard CAN ID, for an RTR frame.\n"
		"%s    <EID>x            - Extended CAN ID.\n"
		"%s    <EID>xr           - Extended CAN identifier, for an RTR frame.\n"
		"\n",
		indent, indent, indent, indent, indent, indent);
}

int fprintCanFrameHelp (FILE* stream, const char* indent)
{
	return fprintf (stream, ""
		"%s<CAN Frame>           - A CAN frame. May be a data frame or RTR frame, based on\n"
		"%s                        the ID. Takes the following format:\n"
		"%s    <CAN ID>[<Byte 0>,<Byte 1>,...<Byte N>]\n"
		"\n"
		"%s<Byte i>              - The i'th byte of a frame's data payload, indexed in\n"
		"%s                        little-endian (aka Intel format). May be either decimal\n"
		"%s                        or hexadecimal (hex should be prefixed with '0x').\n"
		"\n",
		indent, indent, indent, indent, indent, indent);
}
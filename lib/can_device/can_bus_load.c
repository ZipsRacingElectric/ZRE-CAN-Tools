// Header
#include "can_bus_load.h"

// CAN 2.0A frame
#define SOF_LENGTH			1
#define SID_LENGTH			11
#define RTR_LENGTH			1
#define IDE_LENGTH			1
#define R0_LENGTH			1
#define DLC_LENGTH			4
#define CRC_LENGTH			15
#define CRC_DELIM_LENGTH	1
#define ACK_LENGTH			1
#define ACK_DELIM_LENGTH	1
#define EOF_LENGTH			7
#define IFS_LENGTH			3

// CAN 2.0B frame
#define IDE_A_LENGTH		11
#define SRR_LENGTH			1
#define IDE_B_LENGTH		18
#define R0_R1_LENGTH		2

size_t canGetMaxBitCount (canFrame_t* frame)
{
	size_t stuffableBits;
	size_t nonStuffableBits;

	if (!frame->ide)
	{
		// CAN 2.0A frame

		// SID, RTR, IDE, R0, DLC, data, CRC, and ACK are stuffable.
		stuffableBits =
			SID_LENGTH +
			RTR_LENGTH +
			IDE_LENGTH +
			R0_LENGTH +
			DLC_LENGTH +
			8 * frame->dlc +
			CRC_LENGTH +
			ACK_LENGTH;

		// SOF, CRC delim, ACK delim, EOF, and IFS are not stuffable.
		nonStuffableBits =
			SOF_LENGTH +
			CRC_DELIM_LENGTH +
			ACK_DELIM_LENGTH +
			EOF_LENGTH +
			IFS_LENGTH;
	}
	else
	{
		// CAN 2.0B frame

		// IDE A/B, SRR, IDE, RTR, R0, R1, DLC, data, CRC, and ACK are stuffable.
		stuffableBits =
			IDE_A_LENGTH +
			SRR_LENGTH +
			IDE_LENGTH +
			IDE_B_LENGTH +
			RTR_LENGTH +
			R0_R1_LENGTH +
			DLC_LENGTH +
			8 * frame->dlc +
			CRC_LENGTH +
			ACK_LENGTH;

		// SOF, CRC delim, ACK delim, EOF, and IFS are not stuffable.
		nonStuffableBits =
			SOF_LENGTH +
			CRC_DELIM_LENGTH +
			ACK_DELIM_LENGTH +
			EOF_LENGTH +
			IFS_LENGTH;
	}

	// Every 4 consecutive bits may be stuffed. Assume worst case is they all are.
	return stuffableBits + nonStuffableBits + (stuffableBits - 1) / 4;
}

size_t canGetMinBitCount (canFrame_t* frame)
{
	// Min count assumes no bits are stuffed, hence we just use the base size of the frame.

	if (!frame->ide)
	{
		// CAN 2.0A frame
		return
			SOF_LENGTH +
			SID_LENGTH +
			RTR_LENGTH +
			IDE_LENGTH +
			R0_LENGTH +
			DLC_LENGTH +
			8 * frame->dlc +
			CRC_LENGTH +
			CRC_DELIM_LENGTH +
			ACK_LENGTH +
			ACK_DELIM_LENGTH +
			EOF_LENGTH +
			IFS_LENGTH;
	}

	// CAN 2.0B frame
	return
		SOF_LENGTH +
		IDE_A_LENGTH +
		SRR_LENGTH +
		IDE_LENGTH +
		IDE_B_LENGTH +
		RTR_LENGTH +
		R0_R1_LENGTH +
		DLC_LENGTH +
		8 * frame->dlc +
		CRC_LENGTH +
		CRC_DELIM_LENGTH +
		ACK_LENGTH +
		ACK_DELIM_LENGTH +
		EOF_LENGTH +
		IFS_LENGTH;
}

float canCalculateBitTime (canBaudrate_t baudrate)
{
	// Bit time is the inverse of the baudrate. Ex: 1Mbit/s => 1us/bit.
	return 1.0f / baudrate;
}

float canCalculateBusLoad (size_t bitCount, float bitTime, struct timespec period)
{
	// CAN bus load is the amount of time the bus was in use (length of bit * number of bits) divided by the total period of
	// time we measured for.
	float periodS = period.tv_sec + period.tv_nsec * 1e-9f;
	return bitTime * bitCount / periodS;
}
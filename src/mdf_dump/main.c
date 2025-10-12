// MDF Dump -------------------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2025.09.29
//
// Description: Program for performing light parsing and interpretation of MDF files. Used for checking MDF file validity and
//   debugging erroneous implementations.

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "mdf/mdf_reader.h"
#include "debug.h"
#include "error_codes.h"

// C Standard Library
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Functions ------------------------------------------------------------------------------------------------------------------

void asciidump (void* buffer, size_t length, size_t width, FILE* stream)
{
	char* data = buffer;

	size_t count = 0;
	while (length > 0)
	{
		fprintf (stream, "%c ", *data);
		++data;
		--length;
		++count;
		if (count % width == 0 && length != 0)
			fprintf (stream, "\n");
	}
}

void hexdump (void* buffer, size_t length, size_t width, FILE* stream)
{
	uint8_t* data = buffer;

	size_t count = 0;
	while (length > 0)
	{
		fprintf (stream, "%02X ", *data);
		++data;
		--length;
		++count;
		if (count % width == 0 && length != 0)
			fprintf (stream, "\n");
	}
}

void dumpBlockHeader (FILE* stream, mdfBlock_t* block)
{
	// Block ID
	hexdump (&block->header.blockId, sizeof (block->header.blockId), sizeof (block->header.blockId), stream);
	fprintf (stream, "| %s\n", block->header.blockIdString);

	// Block length
	hexdump (&block->header.blockLength, sizeof (block->header.blockLength), sizeof (block->header.blockLength), stream);
	fprintf (stream, "| Block length = %lu\n", block->header.blockLength);

	// Link count
	hexdump (&block->header.linkCount, sizeof (block->header.linkCount), sizeof (block->header.linkCount), stream);
	fprintf (stream, "| Link count = %lu\n\n", block->header.linkCount);
}

void dumpBlockLinkList (FILE* stream, mdfBlock_t* block)
{
	for (size_t linkIndex = 0; linkIndex < block->header.linkCount; ++linkIndex)
	{
		hexdump (&block->linkList [linkIndex], sizeof (uint64_t), sizeof (uint64_t), stream);
		fprintf (stream, "| 0x%08lX\n", block->linkList [linkIndex]);
	}
}

typedef struct
{
	FILE* timestampStream;

	size_t recordIndex;
	uint8_t recordId;

	uint8_t dlc0;
	uint8_t dlc1;
	uint8_t dlc2;

	uint64_t timestamp0;
	uint64_t timestamp1;
	uint32_t canId;
} dataSectionArg_t;

void dataSectionByteHandler (mdfBlock_t* block, uint8_t data, void* dataSectionByteArg)
{
	dataSectionArg_t* arg = dataSectionByteArg;

	switch (block->header.blockId)
	{
	case MDF_BLOCK_ID_DT:
		printf ("%02X ", data);

		// Record ID
		if (arg->recordIndex == 0)
		{
			arg->recordId = data;
			switch (arg->recordId)
			{
			case 1:
				printf ("(CAN RX)  | ");
				break;

			default:
				printf ("(Unknown) | ");
				fprintf (stderr, "Warning: Unknown record ID '0x%04X'.\n", arg->recordId);
				break;
			}
		}

		switch (arg->recordId)
		{
		case 1:
			// Time stamp 0

			if (arg->recordIndex == 2)
				arg->timestamp0 = 0;
			if (2 <= arg->recordIndex && arg->recordIndex < 9)
			{
				arg->timestamp0 |= ((uint64_t) data) << ((arg->recordIndex - 2) * 8);
			}
			if (arg->recordIndex == 8)
			{
				fprintf (arg->timestampStream, "0x%016lX, %lu, ", arg->timestamp0, arg->timestamp0);
				printf ("(Timestamp 0 = %16lu) | ", arg->timestamp0);
			}

			// CAN ID

			if (arg->recordIndex == 9)
				arg->canId = 0;
			if (9 <= arg->recordIndex && arg->recordIndex < 13)
				arg->canId |= data << ((arg->recordIndex - 9) * 8);
			if (arg->recordIndex == 12)
			{
				arg->canId >>= 3;
				printf ("(CAN ID = 0x%03X) | ", arg->canId);
			}

			// DLC 0

			if (arg->recordIndex == 13)
			{
				if (data % 2 != 0)
					fprintf (stderr, "Warning: Fractional DLC 0.\n");

				if (data / 2 <= 8)
				{
					arg->dlc0 = data / 2;
					printf ("(DLC 0 = %u) | ", arg->dlc0);
				}
				else
				{
					arg->dlc0 = 0;
					printf ("(DLC 0 invalid) | ");
					fprintf (stderr, "Warning: Invalid DLC 0 '%u'.\n", data);
				}
			}

			if (arg->recordIndex == 14)
			{
				if (arg->dlc0 * 16 != data)
					fprintf (stderr, "Warning: Mismatched DLC 1, '%u' != '%u'.\n", arg->dlc0 * 16, data);

				if (data / 16 <= 8)
				{
					arg->dlc1 = data / 16;
					printf ("(DLC 1 = %u) | ", arg->dlc1);
				}
				else
				{
					arg->dlc1 = 0;
					printf ("(DLC 1 invalid) | ");
					fprintf (stderr, "Warning: Invalid DLC 1 '%u'.\n", data);
				}
			}

			// Timestamp 1
			if (arg->recordIndex == 14)
				arg->timestamp1 = 0;
			if (14 <= arg->recordIndex && arg->recordIndex < 18)
			{
				arg->timestamp1 |= ((uint64_t) data) << ((arg->recordIndex - 14) * 8);
			}
			if (arg->recordIndex == 17)
			{
				fprintf (arg->timestampStream, "0x%08lX, %lu, \n", arg->timestamp1, arg->timestamp1);
			}

			// DLC

			if (arg->recordIndex == 23)
				printf ("| ");

			if (arg->recordIndex == 24)
			{
				if (data != arg->dlc0)
					fprintf (stderr, "Warning: Mismatched DLC 2, '%u' != '%u'.\n", data, arg->dlc0);

				if (data <= 8)
				{
					arg->dlc2 = data;
					printf ("(DLC 2 = %u) | ", arg->dlc2);
				}
				else
				{
					arg->dlc2 = 0;
					printf ("(DLC invalid) | ");
					fprintf (stderr, "Warning: Invalid DLC 2 '%u'.\n", data);
				}
			}

			// Payload

			if (arg->recordIndex == 27)
				printf ("| ");

			if (arg->recordIndex == 27U + arg->dlc0)
			{
				arg->recordIndex = 0;
				if (arg->dlc0 == 0)
					printf ("(No Payload)\n");
				else
					printf ("(Payload)\n");
				return;
			}
			break;

		default:
			if (arg->recordIndex == 32)
			{
				printf ("\n");
				arg->recordIndex = 0;
				return;
			}
		}
	break;

	case MDF_BLOCK_ID_MD:
	case MDF_BLOCK_ID_TX:
		if (data != '\0')
			printf ("%c", data);
		break;

	default:
		printf ("%02X ", data);

		if (arg->recordIndex % 32 == 31)
			printf ("\n");
		break;
	}

	++arg->recordIndex;
}

int main (int argc, char** argv)
{
	debugInit ();

	FILE* timestampStream = fopen ("/dev/null", "w");
	size_t blockCount = (size_t) -1;

	FILE* mdf = stdin;
	for (int index = 1; index < argc; ++index)
	{
		if (argv [index][0] == '-')
		{
			if (argv [index][1] == 't' && argv [index][2] != '=')
			{
				timestampStream = fopen (argv [index] + 3, "w");
				if (timestampStream == NULL)
				{
					fprintf (stderr, "Failed to open timestamp stream '%s': %s", argv [index] + 3, strerror (errno));
					return errno;
				}
				continue;
			}

			if (argv [index][1] == 'b' && argv [index][2] == '=')
			{
				blockCount = strtoul (argv [index] + 3, NULL, 0);
				if (blockCount == 0)
					return 0;
				continue;
			}
		}

		if (index == argc - 1)
		{
			mdf = fopen (argv [index], "r");
			if (mdf == NULL)
			{
				int code = errno;
				fprintf (stderr, "Failed to open MDF file: %s.\n", errorMessage (code));
				return code;
			}
		}
	}

	printf ("- Begin MDF File -\n\n");

	mdfFileIdBlock_t fileIdBlock;
	if (mdfReadFileIdBlock (mdf, &fileIdBlock) != 0)
	{
		int code = errno;
		fprintf (stderr, "Failed to read MDF file ID block: %s.\n", errorMessage (code));
		return code;
	}

	printf ("- Begin File ID Block -\n\n");
	hexdump (&fileIdBlock, sizeof (fileIdBlock), 8, stdout);
	printf ("\n\n- End File ID Block -\n\n");

	for (size_t count = 0; count < blockCount - 1; ++count)
	{
		if (mdfReaderJumpToBlock (mdf) != 0)
			break;

		mdfBlock_t block;
		if (mdfReadBlockHeader (mdf, &block) != 0)
		{
			int code = errno;
			fprintf (stderr, "Failed to read MDF block header: %s.\n", errorMessage (code));
			return code;
		}

		printf ("- Begin Header - 0x%08lX -\n\n", block.addr);
		dumpBlockHeader (stdout, &block);
		printf ("- End Header -\n\n");

		if (mdfReadBlockLinkList (mdf, &block) != 0)
		{
			int code = errno;
			fprintf (stderr, "Failed to read MDF block link list: %s.\n", errorMessage (code));
			return code;
		}

		size_t linkLinkLength = 8 * block.header.linkCount;
		if (linkLinkLength != 0)
		{
			printf ("- Begin Link List -\n\n");
			dumpBlockLinkList (stdout, &block);
			printf ("\n- End Link List -\n\n");
		}

		printf ("- Begin Data Section -\n\n");

		dataSectionArg_t dataSectionByteArg =
		{
			.timestampStream = timestampStream,
			.recordIndex = 0,
		};

		if (mdfReadBlockDataSection (mdf, &block) != 0)
		{
			int code = errno;
			fprintf (stderr, "Failed to read MDF block data section: %s.\n", errorMessage (code));
			return code;
		}

		size_t dataSectionSize = mdfBlockDataSectionSize (&block);
		for (size_t index = 0; index < dataSectionSize; ++index)
			dataSectionByteHandler (&block, ((uint8_t*) block.dataSection) [index], &dataSectionByteArg);

		if (dataSectionSize == 0)
		{
			int data;
			while (true)
			{
				data = fgetc (mdf);
				if (data == EOF)
					break;
				dataSectionByteHandler (&block, data, &dataSectionByteArg);
			}
		}

		printf ("\n\n- End Data Section -\n\n");

		if (dataSectionSize == 0)
			break;
	}

	printf ("- End MDF File -\n");

	return 0;
}
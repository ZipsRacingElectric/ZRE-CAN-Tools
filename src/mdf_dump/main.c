// MDF Dump -------------------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2025.09.29
//
// Description: Program for performing light parsing and interpretation of MDF files. Used for checking MDF file validity and
//   debugging erroneous implementations.

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "debug.h"
#include "error_codes.h"

// C Standard Library
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Error Codes ----------------------------------------------------------------------------------------------------------------

#define ERRNO_UNKNOWN 2048
#define ERRNO_END_OF_FILE 2049

// Datatypes ------------------------------------------------------------------------------------------------------------------

#define BLOCK_ID_STR(a, b, c, d) (uint32_t)((a) | (b) << 8 | (c) << 16 | (d) << 24)

typedef struct
{
	uint8_t data [64];
} mdfFileIdBlock_t;

typedef enum: uint64_t
{
	BLOCK_ID_DT = BLOCK_ID_STR ('#', '#', 'D', 'T'),
	BLOCK_ID_MD = BLOCK_ID_STR ('#', '#', 'M', 'D'),
	BLOCK_ID_TX = BLOCK_ID_STR ('#', '#', 'T', 'X'),
	BLOCK_ID_UNKNOWN
} blockId_t;

typedef struct
{
	long addr;
	struct
	{
		blockId_t blockId;
		uint64_t blockLength;
		uint64_t linkCount;
	} header;
	uint64_t* linkList;
	uint64_t dataSectionSize;
	uint8_t* dataSection;
} mdfBlock_t;

typedef void (mdfByteHandler_t) (mdfBlock_t* block, uint8_t data, void* arg);

// Functions ------------------------------------------------------------------------------------------------------------------

int handleFreadError (FILE* stream)
{
	if (feof (stream))
		return ERRNO_END_OF_FILE;

	if (errno != 0)
		return errno;

	return ERRNO_UNKNOWN;
}

int skipToBlock (FILE* stream)
{
	// Consume any non '#' characters.
	int c;
	do
	{
		c = getc (stream);
	} while (c != '#' && c != EOF);
	if (c == EOF)
		return ERRNO_END_OF_FILE;

	// Put the character back into the stream.
	ungetc (c, stream);
	return 0;
}

FILE* openMdf (const char* filePath, mdfFileIdBlock_t* fileIdBlock)
{
	// Open the file for reading.
	FILE* stream = fopen (filePath, "r");
	if (stream == NULL)
		return NULL;

	// Read the file ID block
	if (fread (fileIdBlock, sizeof (mdfFileIdBlock_t), 1, stream) != 1)
	{
		errno = handleFreadError (stream);
		return NULL;
	}

	return stream;
}

int readBlockHeader (FILE* stream, mdfBlock_t* block)
{
	// Store the address of the block
	block->addr = ftell (stream);

	// Read the block's header
	if (fread (&block->header, sizeof (block->header), 1, stream) != 1)
		return handleFreadError (stream);

	return 0;
}

int readBlockLinkList (FILE* stream, mdfBlock_t* block)
{
	// Read the block's link list
	block->linkList = malloc (block->header.linkCount * sizeof (uint64_t));
	if (block->linkList == NULL)
		return errno;

	if (fread (block->linkList, sizeof (uint64_t), block->header.linkCount, stream) != block->header.linkCount)
		return handleFreadError (stream);

	return 0;
}

int readBlockDataSection (FILE* stream, mdfBlock_t* block, mdfByteHandler_t* dataSectionByteHandler, void* dataSectionByteArg)
{
	// Read the block's data section. Note that a data section size of zero indicates the block's data section extends to the
	// end of the file.
	block->dataSectionSize = block->header.blockLength - sizeof (block->header) - sizeof (uint64_t) * block->header.linkCount;
	for (size_t index = 0; index < block->dataSectionSize || block->dataSectionSize == 0; ++index)
	{
		int data = fgetc (stream);
		if (data == EOF)
		{
			// If we expected more data, return error.
			if (block->dataSectionSize != 0)
				return handleFreadError (stream);

			break;
		}

		// Pass the received byte to the user provided handler.
		dataSectionByteHandler (block, (uint8_t) data, dataSectionByteArg);
	}

	return 0;
}

void deallocBlock (mdfBlock_t* block)
{
	free (block->dataSection);
	free (block->linkList);
}

void asciidump (void* buffer, size_t length, size_t width, FILE* stream)
{
	size_t count = 0;
	while (length > 0)
	{
		fprintf (stream, "%c ", *((char*) buffer));
		++buffer;
		--length;
		++count;
		if (count % width == 0 && length != 0)
			fprintf (stream, "\n");
	}
}

void hexdump (void* buffer, size_t length, size_t width, FILE* stream)
{
	size_t count = 0;
	while (length > 0)
	{
		fprintf (stream, "%02X ", *((uint8_t*) buffer));
		++buffer;
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
	fprintf (stream, "| ");
	asciidump (&block->header.blockId, 4, 4, stdout);
	printf ("\n");

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
	case BLOCK_ID_DT:
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

	case BLOCK_ID_MD:
	case BLOCK_ID_TX:
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

	if (argc < 2)
	{
		fprintf (stderr, "Invalid arguments, usage: mdf-dump <MDF file path>.\n");
		return -1;
	}

	char* filePath = argv [argc - 1];
	FILE* timestampStream = fopen ("/dev/null", "w");
	size_t blockCount = (size_t) -1;

	for (int index = 1; index < argc - 1; ++index)
	{
		if (argv [index][0] == '-' && argv [index][1] == 't' && argv [index][2] != '=')
		{
			timestampStream = fopen (argv [index] + 3, "w");
			if (timestampStream == NULL)
			{
				fprintf (stderr, "Failed to open timestamp stream '%s': %s", argv [index] + 3, strerror (errno));
				return errno;
			}
		}

		if (argv [index][0] == '-' && argv [index][1] == 'b' && argv [index][2] == '=')
		{
			blockCount = strtoul (argv [index] + 3, NULL, 0);
			if (blockCount == 0)
				return 0;
		}
	}

	printf ("- Begin MDF File -\n\n");

	mdfFileIdBlock_t fileIdBlock;
	FILE* mdf = openMdf (filePath, &fileIdBlock);
	if (mdf == NULL)
	{
		int code = errno;
		fprintf (stderr, "Failed to open MDF file: %s.\n", errorMessage (code));
		return code;
	}

	printf ("- Begin File ID Block -\n\n");
	hexdump (&fileIdBlock, sizeof (fileIdBlock), 8, stdout);
	printf ("\n\n- End File ID Block -\n\n");

	for (size_t count = 0; count < blockCount - 1; ++count)
	{
		if (skipToBlock (mdf) != 0)
			break;

		mdfBlock_t block;
		if (readBlockHeader (mdf, &block) != 0)
		{
			int code = errno;
			fprintf (stderr, "Failed to read MDF block header: %s.\n", errorMessage (code));
			return code;
		}

		printf ("- Begin Header - 0x%08lX -\n\n", block.addr);
		dumpBlockHeader (stdout, &block);
		printf ("- End Header -\n\n");

		if (readBlockLinkList (mdf, &block) != 0)
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

		if (readBlockDataSection (mdf, &block, dataSectionByteHandler, &dataSectionByteArg) != 0)
		{
			int code = errno;
			fprintf (stderr, "Failed to read MDF block data section: %s.\n", errorMessage (code));
			return code;
		}

		printf ("\n\n- End Data Section -\n\n");

		if (block.dataSectionSize == 0)
			break;
	}

	printf ("- End MDF File -\n");

	return 0;
}
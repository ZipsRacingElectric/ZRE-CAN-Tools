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

// C Standard Library
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Datatypes ------------------------------------------------------------------------------------------------------------------

typedef enum size_t
{
	BLOCK_ID_DT,
	BLOCK_ID_MD,
	BLOCK_ID_TX,
	BLOCK_ID_UNKNOWN
} blockId_t;

const char* BLOCK_ID_STRINGS [] =
{
	[BLOCK_ID_DT] = "##DT",
	[BLOCK_ID_MD] = "##MD",
	[BLOCK_ID_TX] = "##TX",
};

// Functions ------------------------------------------------------------------------------------------------------------------

blockId_t checkBlockId (uint8_t* header)
{
	char blockIdString [5];

	memcpy (blockIdString, header, sizeof (blockIdString) - 1);

	blockIdString [sizeof (blockIdString) - 1] = '\0';

	for (size_t index = 0; index < sizeof (BLOCK_ID_STRINGS) / sizeof (char *); ++index)
		if (strcmp (blockIdString, BLOCK_ID_STRINGS [index]) == 0)
			return index;

	fprintf (stderr, "Warning: Unknown block ID string '%s'.\n", blockIdString);
	return BLOCK_ID_UNKNOWN;
}

uint64_t getBlockLength (uint8_t* header)
{
	uint64_t blockLength = 0;
	for (size_t index = 0; index < 8; ++index)
		blockLength |= header [index + 8] << (index * 8);
	return blockLength;
}

uint64_t getLinkCount (uint8_t* header)
{
	uint64_t linkCount = 0;
	for (size_t index = 0; index < 8; ++index)
		linkCount |= header [index + 16] << (index * 8);
	return linkCount;
}

uint64_t getDataCount (uint64_t blockLength, uint64_t linkCount)
{
	return blockLength - 24 - linkCount * 8;
}

int skipToHeader (FILE* stream)
{
	// Consume any non '#' characters.
	int c;
	do
	{
		c = getc (stream);
	} while (c != '#' && c != EOF);
	if (c == EOF)
		return -1;

	ungetc (c, stream);

	return 0;
}

int handleFileError (FILE* stream, const char* message)
{
	if (feof (stream))
	{
		fprintf (stderr, "%s: Unexpected end of file.\n", message);
		return -1;
	}

	int code = errno;
	fprintf (stderr, "%s: %s.\n", message, strerror (code));
	return code;
}

void asciidump (uint8_t* buffer, size_t length, size_t width, FILE* stream)
{
	size_t count = 0;
	while (length > 0)
	{
		fprintf (stream, "%c ", (char) *buffer);
		++buffer;
		--length;
		++count;
		if (count % width == 0 && length != 0)
			fprintf (stream, "\n");
	}
}

void hexdump (uint8_t* buffer, size_t length, size_t width, FILE* stream)
{
	size_t count = 0;
	while (length > 0)
	{
		fprintf (stream, "%02X ", *buffer);
		++buffer;
		--length;
		++count;
		if (count % width == 0 && length != 0)
			fprintf (stream, "\n");
	}
}

void canRecordDump (uint8_t* buffer, size_t length, FILE* stream)
{
	for (size_t index = 0; index < length; ++index)
		printf ("%02X ", buffer [index]);
}

int main (int argc, char** argv)
{
	debugInit ();

	if (argc < 2)
	{
		fprintf (stderr, "Invalid arguments, usage: mdf-dump <MDF file path>.\n");
		return -1;
	}

	char* filename = argv [argc - 1];
	FILE* timestampStream = fopen ("/dev/null", "w");
	size_t blockCount = (size_t) -1;

	for (size_t index = 1; index < argc - 1; ++index)
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

	fpos_t filePointer;
	FILE* mdf = fopen (filename, "r");
	if (mdf == NULL)
	{
		int code = errno;
		fprintf (stderr, "Failed to open file '%s': %s.\n", filename, strerror (code));
		return code;
	}

	printf ("- Begin MDF File -\n\n");

	uint8_t fileId [64];
	fgetpos (mdf, &filePointer);
	if (fread (fileId, 1, sizeof (fileId), mdf) != sizeof (fileId))
		return handleFileError (mdf, "Failed to read file ID block");

	printf ("- Begin File ID Block - 0x%08lX -\n\n", filePointer.__pos);
	hexdump (fileId, sizeof (fileId), 8, stdout);
	printf ("\n\n- End File ID Block -\n\n");

	for (size_t count = 0; count < blockCount - 1; ++count)
	{
		if (skipToHeader (mdf) != 0)
			break;

		uint8_t header [24];

		// Block header
		fgetpos (mdf, &filePointer);
		if (fread (header, 1, sizeof (header), mdf) != sizeof (header))
			return handleFileError (mdf, "Failed to read block header");

		blockId_t blockId = checkBlockId (header);

		printf ("- Begin Header - 0x%08lX -\n\n", filePointer.__pos);
		hexdump (header, 8, 8, stdout);
		printf ("| ");
		asciidump(header, 4, 4, stdout);
		printf ("\n");

		uint64_t blockLength = getBlockLength (header);
		hexdump (header + 8, 8, 8, stdout);
		printf ("| Block length = %lu\n", blockLength);

		uint64_t linkCount = getLinkCount (header);
		hexdump (header + 16, 8, 8, stdout);
		printf ("| Link count = %lu\n\n", linkCount);

		hexdump (header + 24, sizeof (header) - 24, 8, stdout);
		printf ("- End Header -\n\n");

		size_t linkLinkLength = 8 * linkCount;
		if (linkLinkLength != 0)
		{
			uint8_t* linkList = malloc (linkLinkLength);
			fgetpos (mdf, &filePointer);
			if (fread (linkList, 1, linkLinkLength, mdf) != linkLinkLength)
				return handleFileError (mdf, "Failed to read link list");

			printf ("- Begin Link List - 0x%08lX -\n\n", filePointer.__pos);
			for (size_t linkIndex = 0; linkIndex < linkCount; ++linkIndex)
			{
				uint64_t addr = 0;
				for (size_t byteIndex = 0; byteIndex < 8; ++byteIndex)
					addr |= ((uint64_t) linkList [linkIndex * 8 + byteIndex]) << (byteIndex * 8);

				hexdump (linkList + linkIndex * 8, 8, 8, stdout);
				printf ("| 0x%08lX\n", addr);
			}
			printf ("\n- End Link List -\n\n");

			free (linkList);
		}

		fgetpos (mdf, &filePointer);
		printf ("- Begin Data Section - 0x%08lX -\n\n", filePointer.__pos);

		size_t dataLength = getDataCount (blockLength, linkCount);

		enum : uint8_t
		{
			ID_CAN_RX	= 0x01
		} recordId = 0;

		uint8_t dlc0;
		uint8_t dlc1;
		uint8_t dlc2;

		size_t recordIndex = 0;
		size_t dataCount = 0;
		uint64_t timestamp0 = 0;
		uint64_t timestamp1 = 0;
		uint32_t canId = 0;

		while (dataCount < dataLength || dataLength == 0)
		{
			int data = fgetc (mdf);
			if (data == EOF)
			{
				if (dataLength != 0)
					return handleFileError (mdf, "Failed to read data section");

				break;
			}
			++dataCount;

			if (blockId == BLOCK_ID_DT)
			{
				printf ("%02X ", data);

				// Record ID
				if (recordIndex == 0)
					recordId = 0;

				if (recordIndex < sizeof (recordId))
					recordId |= data << (recordIndex * 8);

				if (recordIndex == sizeof (recordId) - 1)
				{
					switch (recordId)
					{
					case ID_CAN_RX:
						printf ("(CAN RX)  | ");
						break;

					default:
						printf ("(Unknown) | ");
						fprintf (stderr, "Warning: Unknown record ID '0x%04X'.\n", recordId);
						break;
					}
				}

				if (recordId == ID_CAN_RX)
				{
					// Time stamp 0

					if (recordIndex == 2)
						timestamp0 = 0;
					if (2 <= recordIndex && recordIndex < 9)
					{
						timestamp0 |= ((uint64_t) data) << ((recordIndex - 2) * 8);
					}
					if (recordIndex == 8)
					{
						fprintf (timestampStream, "0x%016lX, %lu, ", timestamp0, timestamp0);
						printf ("(Timestamp 0 = %16lu) | ", timestamp0);
					}

					// CAN ID

					if (recordIndex == 9)
						canId = 0;
					if (9 <= recordIndex && recordIndex < 13)
						canId |= data << ((recordIndex - 9) * 8);
					if (recordIndex == 12)
					{
						canId >>= 3;
						printf ("(CAN ID = 0x%03X) | ", canId);
					}

					// DLC 0

					if (recordIndex == 13)
					{
						if (data % 2 != 0)
							fprintf (stderr, "Warning: Fractional DLC 0.\n");

						if (data / 2 >= 0 && data / 2 <= 8)
						{
							dlc0 = data / 2;
							printf ("(DLC 0 = %u) | ", dlc0);
						}
						else
						{
							dlc0 = 0;
							printf ("(DLC 0 invalid) | ");
							fprintf (stderr, "Warning: Invalid DLC 0 '%u'.\n", data);
						}
					}

					if (recordIndex == 14)
					{
						if (dlc0 * 16 != data)
							fprintf (stderr, "Warning: Mismatched DLC 1, '%u' != '%u'.\n", dlc0 * 16, data);

						if (data / 16 >= 0 && data / 16 <= 8)
						{
							dlc1 = data / 16;
							printf ("(DLC 1 = %u) | ", dlc1);
						}
						else
						{
							dlc1 = 0;
							printf ("(DLC 1 invalid) | ");
							fprintf (stderr, "Warning: Invalid DLC 1 '%u'.\n", data);
						}
					}

					// Timestamp 1
					if (recordIndex == 14)
						timestamp1 = 0;
					if (14 <= recordIndex && recordIndex < 18)
					{
						timestamp1 |= ((uint64_t) data) << ((recordIndex - 14) * 8);
					}
					if (recordIndex == 17)
					{
						fprintf (timestampStream, "0x%08lX, %lu, \n", timestamp1, timestamp1);
					}

					// DLC

					if (recordIndex == 23)
						printf ("| ");

					if (recordIndex == 24)
					{
						if (data != dlc0)
							fprintf (stderr, "Warning: Mismatched DLC 2, '%u' != '%u'.\n", data, dlc0);

						if (data >= 0 && data <= 8)
						{
							dlc2 = data;
							printf ("(DLC 2 = %u) | ", dlc2);
						}
						else
						{
							dlc2 = 0;
							printf ("(DLC invalid) | ");
							fprintf (stderr, "Warning: Invalid DLC 2 '%u'.\n", data);
						}
					}

					// Payload

					if (recordIndex == 27)
						printf ("| ");

					if (recordIndex == 27 + dlc0)
					{
						recordIndex = 0;
						if (dlc0 == 0)
							printf ("(No Payload)\n");
						else
							printf ("(Payload)\n");
						continue;
					}
				}
				else
				{
					if (recordIndex == 32)
					{
						printf ("\n");
						recordIndex = 0;
						continue;
					}
				}

				++recordIndex;
			}
			else if (blockId == BLOCK_ID_MD || blockId == BLOCK_ID_TX)
			{
				if (data != '\0')
					printf ("%c", data);
			}
			else
			{
				printf ("%02X ", data);

				if (dataCount % 32 == 0)
					printf ("\n");
			}
		}

		printf ("\n\n- End Data Section -\n\n");

		if (dataLength == 0)
			break;
	}

	printf ("- End MDF File -\n");

	return 0;
}
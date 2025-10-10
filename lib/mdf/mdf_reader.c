// Header
#include "mdf_reader.h"
#include "error_codes.h"

// C Standard Library
#include <errno.h>
#include <stdlib.h>

/**
 * @brief Handles an unsuccessful call to @c fread .
 * @param stream The @c FILE* that caused the failure.
 * @return The error code indicating what occurred.
 */
int handleFreadError (FILE* stream);

FILE* mdfReaderOpen (const char* filePath, mdfFileIdBlock_t* fileIdBlock)
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

int mdfReaderSkipToBlock (FILE* stream)
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

int mdfReadBlockHeader (FILE* stream, mdfBlock_t* block)
{
	// Store the address of the block
	block->addr = ftell (stream);

	// Read the block's header
	if (fread (&block->header, sizeof (block->header), 1, stream) != 1)
		return handleFreadError (stream);

	return 0;
}

int mdfReadBlockLinkList (FILE* stream, mdfBlock_t* block)
{
	// Read the block's link list
	block->linkList = malloc (block->header.linkCount * sizeof (uint64_t));
	if (block->linkList == NULL)
		return errno;

	if (fread (block->linkList, sizeof (uint64_t), block->header.linkCount, stream) != block->header.linkCount)
		return handleFreadError (stream);

	return 0;
}

int mdfReadBlockDataSection (FILE* stream, mdfBlock_t* block, mdfByteHandler_t* dataSectionByteHandler, void* dataSectionByteArg)
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

void mdrReaderDeallocBlock (mdfBlock_t* block)
{
	free (block->linkList);
}

int handleFreadError (FILE* stream)
{
	// If the file ended, return the code for that.
	if (feof (stream))
		return ERRNO_END_OF_FILE;

	// If errno is set, that was the error.
	if (errno != 0)
		return errno;

	// No information about what happened. Shouldn't ever occur, but just in case.
	return ERRNO_UNKNOWN;
}
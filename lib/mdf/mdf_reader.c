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
static int handleFreadError (FILE* stream);

int mdfReadFileIdBlock (FILE* mdf, mdfFileIdBlock_t* fileIdBlock)
{
	// Read the file ID block
	if (fread (fileIdBlock, sizeof (*fileIdBlock), 1, mdf) != 1)
		return handleFreadError (mdf);

	return 0;
}

int mdfReadBlockHeader (FILE* mdf, mdfBlock_t* block)
{
	// Store the address of the block
	block->addr = ftell (mdf);

	// Read the block's header
	if (fread (&block->header, sizeof (block->header), 1, mdf) != 1)
		return handleFreadError (mdf);

	// Terminate the block ID string
	block->header.blockIdString [sizeof (block->header.blockIdString) - 1] = '\0';

	return 0;
}

int mdfReadBlockLinkList (FILE* mdf, mdfBlock_t* block)
{
	if (block->header.linkCount == 0)
		return 0;

	// Allocate memory for the list
	block->linkList = malloc (block->header.linkCount * sizeof (uint64_t));
	if (block->linkList == NULL)
		return errno;

	// Read the list
	if (fread (block->linkList, sizeof (uint64_t), block->header.linkCount, mdf) != block->header.linkCount)
		return handleFreadError (mdf);

	return 0;
}

int mdfReadBlockDataSection (FILE* mdf, mdfBlock_t* block)
{
	size_t dataSectionSize = mdfBlockDataSectionSize (block);
	if (dataSectionSize == 0)
		return 0;

	// Allocate memory for the data section
	block->dataSection = malloc (dataSectionSize);
	if (block->dataSection == NULL)
		return errno;

	// Read the data section
	if (fread (block->dataSection, 1, dataSectionSize, mdf) != dataSectionSize)
		return handleFreadError (mdf);

	// If this is a text section, terminate the string
	if (block->header.blockId == MDF_BLOCK_ID_TX)
		((char*) block->dataSection) [dataSectionSize - 1] = '\0';

	return 0;
}

void mdrReaderDeallocBlock (mdfBlock_t* block)
{
	free (block->linkList);
}

int mdfReaderJumpToBlock (FILE* mdf)
{
	// Consume any non '#' characters.
	int c;
	do
	{
		c = getc (mdf);
	} while (c != '#' && c != EOF);
	if (c == EOF)
	{
		errno = ERRNO_END_OF_FILE;
		return errno;
	}

	// Put the character back into the stream.
	if (ungetc (c, mdf) == EOF)
		return errno;

	return 0;
}

static int handleFreadError (FILE* stream)
{
	// If the file ended, return the code for that.
	if (feof (stream))
	{
		errno = ERRNO_END_OF_FILE;
		return errno;
	}

	// If errno is set, that was the error.
	if (errno != 0)
		return errno;

	// No information about what happened. Shouldn't ever occur, but just in case.
	errno = ERRNO_UNKNOWN;
	return errno;
}
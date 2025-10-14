// Header
#include "mdf_block.h"

// C Standard Library
#include <errno.h>
#include <stdlib.h>

int mdfBlockInit (mdfBlock_t* block, uint64_t blockId, uint64_t linkCount, uint64_t dataSectionSize)
{
	block->header.blockId = blockId;
	block->header.linkCount = linkCount;
	block->header.blockLength = dataSectionSize + sizeof (block->header) + sizeof (uint64_t) * linkCount;

	return mdfBlockInitHeader (block);
}

int mdfBlockInitHeader (mdfBlock_t* block)
{
	// Terminate the block ID string if it isn't already
	block->header.blockIdString [sizeof (block->header.blockIdString) - 1] = '\0';

	// Allocate the block's link list
	if (block->header.linkCount != 0)
	{
		block->linkList = calloc (block->header.linkCount, sizeof (uint64_t));
		if (block->linkList == NULL)
			return errno;
	}

	// Allocate the block's data section
	uint64_t dataSectionSize = mdfBlockDataSectionSize (block);
	if (dataSectionSize != 0)
	{
		block->dataSection = calloc (dataSectionSize, 1);
		if (block->dataSection == NULL)
			return errno;
	}

	return 0;
}

void mdfBlockDealloc (mdfBlock_t* block)
{
	// Deallocate the data section
	if (block->dataSection != NULL)
		free (block->dataSection);

	// Deallocate the link list
	if (block->linkList != NULL)
		free (block->linkList);
}
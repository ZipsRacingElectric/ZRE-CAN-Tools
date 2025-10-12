// Header
#include "mdf_writer.h"

// C Standard Library
#include <errno.h>

/**
 * @brief Aligns the I/O stream to an 8 byte address.
 * @param mdf The stream to align.
 * @return 0 if successful, the error code otherwise.
 */
static int alignBlock (FILE* mdf);

int mdfWriteFileIdBlock (FILE* mdf, mdfFileIdBlock_t* fileIdBlock)
{
	if (fwrite (fileIdBlock, sizeof (*fileIdBlock), 1, mdf) != 1)
		return errno;

	return 0;
}

int mdfWriteBlock (FILE* mdf, mdfBlock_t* block, void* dataSection)
{
	// Align the stream for the next block.
	if (alignBlock (mdf) != 0)
		return errno;

	// Set the block's address based on the stream position.
	long addr = ftell (mdf);
	if (addr < 0)
		return errno;
	block->addr = addr;

	// Write the header
	if (fwrite (&block->header, sizeof (block->header), 1, mdf) != 1)
		return errno;

	// Write the link list
	if (block->header.linkCount != 0)
		if (fwrite (block->linkList, sizeof (uint64_t), block->header.linkCount, mdf) != block->header.linkCount)
			return errno;

	// Write the data section
	size_t dataSectionSize = mdfBlockDataSectionSize (block);
	if (dataSectionSize != 0)
		if (fwrite (dataSection, 1, dataSectionSize, mdf) != dataSectionSize)
			return errno;

	return 0;
}

int mdfUpdateBlockLinkList (FILE* mdf, mdfBlock_t* block)
{
	// Jump to the block's link list section
	fseek (mdf, block->addr + sizeof (block->header), SEEK_SET);

	// Write the link list
	if (block->header.linkCount != 0)
		if (fwrite (block->linkList, sizeof (uint64_t), block->header.linkCount, mdf) != block->header.linkCount)
			return errno;
}

static int alignBlock (FILE* mdf)
{
	// Get the current address of the stream.
	long addrCurrent = ftell (mdf);
	if (addrCurrent < 0)
		return errno;

	// Pad the stream with 0's until the next multiple of 8 bytes.
	long padding = addrCurrent % 8;
	uint8_t buffer [] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	if (padding != 0)
		if (fwrite (buffer, sizeof (uint8_t), padding, mdf) != padding)
			return errno;

	return 0;
}
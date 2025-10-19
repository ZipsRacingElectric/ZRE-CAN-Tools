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

uint64_t mdfBlockWrite (FILE* mdf, mdfBlock_t* block)
{
	// Align the stream for the next block.
	if (alignBlock (mdf) != 0)
		return 0;

	// Set the block's address based on the stream position.
	long addr = ftell (mdf);
	if (addr < 0)
		return 0;
	block->addr = addr;

	// Write the header
	if (fwrite (&block->header, sizeof (block->header), 1, mdf) != 1)
		return 0;

	// Write the link list
	if (block->header.linkCount != 0)
		if (fwrite (block->linkList, sizeof (uint64_t), block->header.linkCount, mdf) != block->header.linkCount)
			return 0;

	// Write the data section
	size_t dataSectionSize = mdfBlockDataSectionSize (block);
	if (dataSectionSize != 0)
		if (fwrite (block->dataSection, 1, dataSectionSize, mdf) != dataSectionSize)
			return 0;

	return block->addr;
}

int mdfRewriteBlockLinkList (FILE* mdf, mdfBlock_t* block)
{
	long addr = ftell (mdf);
	if (addr < 0)
		return errno;

	// Jump to the block's link list section
	fseek (mdf, block->addr + sizeof (block->header), SEEK_SET);

	// Write the link list
	if (block->header.linkCount != 0)
		if (fwrite (block->linkList, sizeof (uint64_t), block->header.linkCount, mdf) != block->header.linkCount)
			return errno;

	if (fseek (mdf, addr, SEEK_SET) != 0)
		return errno;

	return 0;
}

int mdfRewriteBlockDataSection (FILE* mdf, mdfBlock_t* block)
{
	size_t dataSectionSize = mdfBlockDataSectionSize (block);

	// Jump to the block's data section
	fseek (mdf, block->addr + sizeof (block->header) + dataSectionSize, SEEK_SET);

	// Write the data section
	if (dataSectionSize != 0)
		if (fwrite (block->linkList, 1, dataSectionSize, mdf) != dataSectionSize)
			return errno;

	return 0;
}

uint64_t mdfCgBlockWrite (FILE* mdf, mdfCgDataSection_t* dataSection, mdfCgLinkList_t* linkList)
{
	mdfBlock_t block;
	mdfCgBlockInit (&block, dataSection, linkList);
	uint64_t addr = mdfBlockWrite (mdf, &block);
	mdfBlockDealloc (&block);

	return addr;
}

uint64_t mdfTxBlockWrite (FILE* mdf, const char* text)
{
	mdfBlock_t block;
	mdfTxBlockInit (&block, text);
	uint64_t addr = mdfBlockWrite (mdf, &block);
	mdfBlockDealloc (&block);
	return addr;
}

uint64_t mdfMdBlockWrite (FILE* mdf, const char* xml)
{
	mdfBlock_t block;
	mdfMdBlockInit (&block, xml);
	uint64_t addr = mdfBlockWrite (mdf, &block);
	mdfBlockDealloc (&block);
	return addr;
}

uint64_t mdfCnBlockWrite (FILE* mdf, mdfCnDataSection_t* dataSection, mdfCnLinkList_t* linkList)
{
	mdfBlock_t block;
	mdfCnBlockInit (&block, dataSection, linkList);
	uint64_t addr = mdfBlockWrite (mdf, &block);
	mdfBlockDealloc (&block);

	return addr;
}

uint64_t mdfCcBlockWrite (FILE* mdf, mdfCcDataSection_t* dataSection, mdfCcLinkList_t* linkList)
{
	mdfBlock_t block;
	mdfCcBlockInit (&block, dataSection, linkList);
	uint64_t addr = mdfBlockWrite (mdf, &block);
	mdfBlockDealloc (&block);

	return addr;
}

uint64_t mdfSiBlockWrite (FILE* mdf, mdfSiDataSection_t* dataSection, mdfSiLinkList_t* linkList)
{
	mdfBlock_t block;
	mdfSiBlockInit (&block, dataSection, linkList);
	uint64_t addr = mdfBlockWrite (mdf, &block);
	mdfBlockDealloc (&block);

	return addr;
}

uint64_t mdfDtBlockWrite (FILE* mdf)
{
	mdfBlock_t block;
	mdfDtBlockInit (&block);
	uint64_t addr = mdfBlockWrite (mdf, &block);
	mdfBlockDealloc (&block);

	return addr;
}

uint64_t mdfDgBlockWrite (FILE* mdf, mdfDgLinkList_t* linkList)
{
	mdfBlock_t block;
	mdfDgBlockInit (&block, linkList);
	uint64_t addr = mdfBlockWrite (mdf, &block);
	mdfBlockDealloc (&block);

	return addr;
}

uint64_t mdfFhBlockWrite (FILE* mdf, mdfFhDataSection_t* dataSection, mdfFhLinkList_t* linkList)
{
	mdfBlock_t block;
	mdfFhBlockInit (&block, dataSection, linkList);
	uint64_t addr = mdfBlockWrite (mdf, &block);
	mdfBlockDealloc (&block);

	return addr;
}

static int alignBlock (FILE* mdf)
{
	// Get the current address of the stream.
	long addrCurrent = ftell (mdf);
	if (addrCurrent < 0)
		return errno;

	// Pad the stream with 0's until the next multiple of 8 bytes.
	size_t padding = (8 - (addrCurrent % 8)) % 8;
	uint8_t buffer [] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	if (padding != 0)
		if (fwrite (buffer, sizeof (uint8_t), padding, mdf) != padding)
			return errno;

	return 0;
}
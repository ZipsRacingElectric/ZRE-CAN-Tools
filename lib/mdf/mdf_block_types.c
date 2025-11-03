// Header
#include "mdf_block_types.h"

// C Standard Library
#include <errno.h>
#include <stdio.h>
#include <string.h>

int mdfHdBlockInit (mdfBlock_t* block, mdfHdDataSection_t* dataSection, mdfHdLinkList_t* linkList)
{
	// Initialize the block
	if (mdfBlockInit (block, MDF_BLOCK_ID_HD, sizeof (*linkList) / sizeof (uint64_t), sizeof (*dataSection)) != 0)
		return errno;

	// Populate the link list and data section
	memcpy (block->linkList, linkList, sizeof (*linkList));
	memcpy (block->dataSection, dataSection, sizeof (*dataSection));

	return 0;
}

int mdfCnBlockInit (mdfBlock_t* block, mdfCnDataSection_t* dataSection, mdfCnLinkList_t* linkList)
{
	// Initialize the block
	if (mdfBlockInit (block, MDF_BLOCK_ID_CN, sizeof (*linkList) / sizeof (uint64_t), sizeof (mdfCnDataSection_t)) != 0)
		return errno;

	// Populate the link list and data section
	memcpy (block->linkList, linkList, sizeof (*linkList));
	memcpy (block->dataSection, dataSection, sizeof (*dataSection));
	return 0;
}

int mdfCgBlockInit (mdfBlock_t* block, mdfCgDataSection_t* dataSection, mdfCgLinkList_t* linkList)
{
	// Initialize the block
	if (mdfBlockInit (block, MDF_BLOCK_ID_CG, sizeof (*linkList) / sizeof (uint64_t), sizeof (*dataSection)) != 0)
		return errno;

	// Populate the link list and data section
	memcpy (block->linkList, linkList, sizeof (*linkList));
	memcpy (block->dataSection, dataSection, sizeof (*dataSection));
	return 0;
}

int mdfCcBlockInit (mdfBlock_t* block, mdfCcDataSection_t* dataSection, mdfCcLinkList_t* linkList)
{
	// Initialize the block
	if (mdfBlockInit (block, MDF_BLOCK_ID_CC, sizeof (*linkList) / sizeof (uint64_t), sizeof (*dataSection)) != 0)
		return errno;

	// Populate the link list and data section
	memcpy (block->linkList, linkList, sizeof (*linkList));
	memcpy (block->dataSection, dataSection, sizeof (*dataSection));

	return 0;
}

int mdfDgBlockInit (mdfBlock_t* block, mdfDgDataSection_t* dataSection, mdfDgLinkList_t* linkList)
{
	// Initialize the block
	if (mdfBlockInit (block, MDF_BLOCK_ID_DG, sizeof (*linkList) / sizeof (uint64_t), sizeof (uint64_t)) != 0)
		return errno;

	// Populate the link list and data section
	memcpy (block->linkList, linkList, sizeof (*linkList));
	memcpy (block->dataSection, dataSection, sizeof (*dataSection));

	return 0;
}

int mdfDtBlockInit (mdfBlock_t* block)
{
	// TODO(Barach): This isn't really accurate. Data blocks can have finite-sized sections.

	// Initialize the block
	return mdfBlockInit (block, MDF_BLOCK_ID_DT, 0, 0);
}

int mdfSiBlockInit (mdfBlock_t* block, mdfSiDataSection_t* dataSection, mdfSiLinkList_t* linkList)
{
	// Initialize the block
	if (mdfBlockInit (block, MDF_BLOCK_ID_SI, sizeof (*linkList) / sizeof (uint64_t), sizeof (*dataSection)) != 0)
		return errno;

	// Populate the link list and data section
	memcpy (block->linkList, linkList, sizeof (*linkList));
	memcpy (block->dataSection, dataSection, sizeof (*dataSection));

	return 0;
}

int mdfFhBlockInit (mdfBlock_t* block, mdfFhDataSection_t* dataSection, mdfFhLinkList_t* linkList)
{
	// Initialize the block
	if (mdfBlockInit (block, MDF_BLOCK_ID_FH, sizeof (*linkList) / sizeof (uint64_t), sizeof (*dataSection)) != 0)
		return errno;

	// Populate the link list and data section
	memcpy (block->linkList, linkList, sizeof (*linkList));
	memcpy (block->dataSection, dataSection, sizeof (*dataSection));

	return 0;
}

int mdfTxBlockInit (mdfBlock_t* block, const char* text, ...)
{
	// Call the variadic version of this function with the given args.
	va_list args;
	va_start (args, text);
	int code = mdfTxBlockInitVariadic (block, text, args);
	va_end (args);

	return code;
}

int mdfTxBlockInitVariadic (mdfBlock_t* block, const char* text, va_list args)
{
	// Copy the args for reuse
	va_list argsCopy;
	va_copy (argsCopy, args);

	// Get the size of the data section based on the format string and args (plus terminator).
	size_t dataSectionSize = vsnprintf (NULL, 0, text, args) + 1;

	// Initialize the block
	if (mdfBlockInit (block, MDF_BLOCK_ID_TX, 0, dataSectionSize) != 0)
		return errno;

	// Populate the data section with the formatted string.
	if (vsprintf (block->dataSection, text, argsCopy) < 0)
		return errno;

	return 0;
}

int mdfMdBlockInit (mdfBlock_t* block, const char* xml, ...)
{
	// Call the variadic version of this function with the given args.
	va_list args;
	va_start (args, xml);
	int code = mdfMdBlockInitVariadic (block, xml, args);
	va_end (args);

	return code;
}

int mdfMdBlockInitVariadic (mdfBlock_t* block, const char* xml, va_list args)
{
	// Copy the args for reuse
	va_list argsCopy;
	va_copy (argsCopy, args);

	// Get the size of the data section based on the format string and args (plus terminator).
	size_t dataSectionSize = vsnprintf (NULL, 0, xml, args) + 1;

	// Initialize the block
	if (mdfBlockInit (block, MDF_BLOCK_ID_MD, 0, dataSectionSize) != 0)
		return errno;

	// Populate the data section with the formatted string.
	if (vsprintf (block->dataSection, xml, argsCopy) < 0)
		return errno;

	return 0;
}
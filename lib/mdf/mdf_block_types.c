// Header
#include "mdf_block_types.h"

// C Standard Library
#include <errno.h>
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

int mdfDgBlockInit (mdfBlock_t* block, mdfDgLinkList_t* linkList)
{
	typedef struct
	{
		uint8_t recordIdLength;
		uint8_t reserved0 [7];
	} mdfDgDataSection_t;

	mdfDgDataSection_t dataSection =
	{
		// TODO(Barach): Can we let this be variable?
		.recordIdLength = 1
	};

	// Initialize the block
	if (mdfBlockInit (block, MDF_BLOCK_ID_DG, sizeof (*linkList) / sizeof (uint64_t), sizeof (uint64_t)) != 0)
		return errno;

	// Populate the link list and data section
	memcpy (block->linkList, linkList, sizeof (*linkList));
	memcpy (block->dataSection, &dataSection, sizeof (dataSection));

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

int mdfTxBlockInit (mdfBlock_t* block, const char* text)
{
	// Data section is 1:1 with the string
	size_t dataSectionSize = strlen (text) + 1;

	// Initialize the block
	if (mdfBlockInit (block, MDF_BLOCK_ID_TX, 0, dataSectionSize) != 0)
		return errno;

	// Populate the data section
	memcpy (block->dataSection, text, dataSectionSize);

	return 0;
}

int mdfMdBlockInit (mdfBlock_t* block, const char* xml)
{
	// Data section is 1:1 with the string
	size_t dataSectionSize = strlen (xml) + 1;

	// Initialize the block
	if (mdfBlockInit (block, MDF_BLOCK_ID_MD, 0, dataSectionSize) != 0)
		return errno;

	// Populate the data section
	memcpy (block->dataSection, xml, dataSectionSize);

	return 0;
}
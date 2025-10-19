#ifndef MDF_WRITER_H
#define MDF_WRITER_H

// MDF Writer -----------------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2025.10.12
//
// Description: Functions and datatypes related to writing to MDF files.
//
// References:
// - https://www.asam.net/standards/detail/mdf/wiki/
// - https://asammdf.readthedocs.io/en/latest/v4blocks.html

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "mdf_block_types.h"

// C Standard Library
#include <stdio.h>

// Functions ------------------------------------------------------------------------------------------------------------------

int mdfWriteFileIdBlock (FILE* mdf, mdfFileIdBlock_t* fileIdBlock);

uint64_t mdfBlockWrite (FILE* mdf, mdfBlock_t* block);

int mdfRewriteBlockLinkList (FILE* mdf, mdfBlock_t* block);

int mdfRewriteBlockDataSection (FILE* mdf, mdfBlock_t* block);

uint64_t mdfCgBlockWrite (FILE* mdf, mdfCgDataSection_t* dataSection, mdfCgLinkList_t* linkList);

uint64_t mdfTxBlockWrite (FILE* mdf, const char* text);

uint64_t mdfMdBlockWrite (FILE* mdf, const char* xml);

uint64_t mdfCnBlockWrite (FILE* mdf, mdfCnDataSection_t* dataSection, mdfCnLinkList_t* linkList);

uint64_t mdfCcBlockWrite (FILE* mdf, mdfCcDataSection_t* dataSection, mdfCcLinkList_t* linkList);

uint64_t mdfSiBlockWrite (FILE* mdf, mdfSiDataSection_t* dataSection, mdfSiLinkList_t* linkList);

uint64_t mdfDtBlockWrite (FILE* mdf);

uint64_t mdfDgBlockWrite (FILE* mdf, mdfDgLinkList_t* linkList);

uint64_t mdfFhBlockWrite (FILE* mdf, mdfFhDataSection_t* dataSection, mdfFhLinkList_t* linkList);

#endif // MDF_WRITER_H
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
#include "mdf_block.h"

// C Standard Library
#include <stdio.h>

// Functions ------------------------------------------------------------------------------------------------------------------

int mdfWriteFileIdBlock (FILE* mdf, mdfFileIdBlock_t* fileIdBlock);

uint64_t mdfBlockWrite (FILE* mdf, mdfBlock_t* block);

// TODO(Barach): Do we need this?
int mdfRewriteBlockHeader (FILE* mdf, mdfBlock_t* block);

int mdfRewriteBlockLinkList (FILE* mdf, mdfBlock_t* block);

int mdfRewriteBlockDataSection (FILE* mdf, mdfBlock_t* block);

#endif // MDF_WRITER_H
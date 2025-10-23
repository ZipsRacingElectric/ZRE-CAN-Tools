#ifndef MDF_READER_H
#define MDF_READER_H

// MDF Reader -----------------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2025.10.09
//
// Description: Functions and datatypes related to reading MDF files. // TODO(Barach): Better description
//
// References:
// - https://www.asam.net/standards/detail/mdf/wiki/
// - https://asammdf.readthedocs.io/en/latest/v4blocks.html

// TODO(Barach):
// - Should probably replace 3 separate functions.

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "mdf_block.h"

// C Standard Library
#include <stdio.h>

// Functions ------------------------------------------------------------------------------------------------------------------

int mdfReadFileIdBlock (FILE* mdf, mdfFileIdBlock_t* fileIdBlock);

int mdfReadBlockHeader (FILE* mdf, mdfBlock_t* block);

int mdfReadBlockLinkList (FILE* mdf, mdfBlock_t* block);

int mdfReadBlockDataSection (FILE* mdf, mdfBlock_t* block);

int mdfReaderJumpToBlock (FILE* mdf);

#endif // MDF_READER_H
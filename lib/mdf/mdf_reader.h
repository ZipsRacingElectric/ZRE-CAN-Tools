// MDF Reader -----------------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2025.10.09
//
// Description: Functions and datatypes related to reading MDF files.
//
// References:
// - https://www.asam.net/standards/detail/mdf/wiki/

// TODO(Barach):
// - Any reason to keep read split into 3 sections?

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
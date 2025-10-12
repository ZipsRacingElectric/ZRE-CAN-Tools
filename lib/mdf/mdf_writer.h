// MDF Writer -----------------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2025.10.12
//
// Description: Functions and datatypes related to writing to MDF files.
//
// References:
// - https://www.asam.net/standards/detail/mdf/wiki/

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "mdf_block.h"

// C Standard Library
#include <stdio.h>

// Functions ------------------------------------------------------------------------------------------------------------------

int mdfWriteFileIdBlock (FILE* mdf, mdfFileIdBlock_t* fileIdBlock);

int mdfWriteBlock (FILE* mdf, mdfBlock_t* block, void* dataSection);
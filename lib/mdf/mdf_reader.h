// MDF Reader -----------------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2025.10.09
//
// Description: Functions and datatypes related to reading MDF files.
//
// References:
// - https://www.asam.net/standards/detail/mdf/wiki/

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "mdf_block.h"

// C Standard Library
#include <stdio.h>

// Datatypes ------------------------------------------------------------------------------------------------------------------

typedef void (mdfByteHandler_t) (mdfBlock_t* block, uint8_t data, void* arg);

// Functions ------------------------------------------------------------------------------------------------------------------

int mdfReadFileIdBlock (FILE* stream, mdfFileIdBlock_t* fileIdBlock);

int mdfReaderSkipToBlock (FILE* stream);

int mdfReadBlockHeader (FILE* stream, mdfBlock_t* block);

int mdfReadBlockLinkList (FILE* stream, mdfBlock_t* block);

int mdfReadBlockDataSection (FILE* stream, mdfBlock_t* block, mdfByteHandler_t* dataSectionByteHandler, void* dataSectionByteArg);

void mdrReaderDeallocBlock (mdfBlock_t* block);
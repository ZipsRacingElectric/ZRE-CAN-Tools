// MDF Block ------------------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2025.10.09
//
// Description: Datatypes and definitions related to MDF blocks.
//
// References:
// - https://www.asam.net/standards/detail/mdf/wiki/

// Includes -------------------------------------------------------------------------------------------------------------------

// C Standard Library
#include <stdint.h>

// Datatypes ------------------------------------------------------------------------------------------------------------------

#define MDF_BLOCK_ID_STR(b0, b1, b2, b3) (uint32_t)((b0) | (b1) << 8 | (b2) << 16 | (b3) << 24)

/// @brief Datatype representing the MDF file ID block, a unique MDF block occurring only once per file.
typedef struct
{
	uint8_t data [64];
} mdfFileIdBlock_t;

/// @brief Datatype representing an MDF block ID.
typedef enum: uint64_t
{
	BLOCK_ID_DT = MDF_BLOCK_ID_STR ('#', '#', 'D', 'T'),
	BLOCK_ID_MD = MDF_BLOCK_ID_STR ('#', '#', 'M', 'D'),
	BLOCK_ID_TX = MDF_BLOCK_ID_STR ('#', '#', 'T', 'X'),
	BLOCK_ID_UNKNOWN
} mdfBlockId_t;

/// @brief Datatype representing an MDF block.
typedef struct
{
	/// @brief The file address of the header of the block.
	long addr;

	/// @brief The block's header.
	struct
	{
		/// @brief The ID of the block.
		mdfBlockId_t blockId;

		/// @brief The length of the block, including the header, in bytes.
		uint64_t blockLength;

		/// @brief The number of addresses in the block's link list.
		uint64_t linkCount;
	} header;

	/// @brief The block's link list. This is an array of file addresses. The number of elements is given by
	/// @c header.linkCount .
	uint64_t* linkList;

	/// @brief The size of the block's data section, in bytes.
	uint64_t dataSectionSize;
} mdfBlock_t;
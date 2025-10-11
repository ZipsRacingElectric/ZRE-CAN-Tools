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

#define MDF_BLOCK_ID_STR_TO_VALUE(c0, c1, c2, c3) ((c0) | (c1) << 8 | (c2) << 16 | (c3) << 24)

#define MDF_BLOCK_ID_DT MDF_BLOCK_ID_STR_TO_VALUE ('#', '#', 'D', 'T')
#define MDF_BLOCK_ID_MD MDF_BLOCK_ID_STR_TO_VALUE ('#', '#', 'M', 'D')
#define MDF_BLOCK_ID_TX MDF_BLOCK_ID_STR_TO_VALUE ('#', '#', 'T', 'X')

/// @brief Datatype representing the MDF file ID block, a unique MDF block occurring only once per file.
typedef struct
{
	uint8_t data [64];
} mdfFileIdBlock_t;

/// @brief Datatype representing an MDF block.
typedef struct
{
	/// @brief The file address of the header of the block.
	long addr;

	/// @brief The block's header.
	struct
	{
		/// @brief The ID of the block.
		uint64_t blockId;

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

// Functions ------------------------------------------------------------------------------------------------------------------

/**
 * @brief Converts a block ID into a string representation.
 * @param blockId The block ID to convert.
 * @return The string representation.
 */
char* mdfBlockIdToString (uint64_t blockId);
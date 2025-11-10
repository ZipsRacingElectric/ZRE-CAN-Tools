#ifndef MDF_BLOCK_H
#define MDF_BLOCK_H

// MDF Block ------------------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2025.10.09
//
// Description: Datatypes and definitions related to MDF blocks. // TODO(Barach): Better description
//
// References:
// - https://www.asam.net/standards/detail/mdf/wiki/
// - https://asammdf.readthedocs.io/en/latest/v4blocks.html

// Includes -------------------------------------------------------------------------------------------------------------------

// C Standard Library
#include <stdint.h>

// File ID Block --------------------------------------------------------------------------------------------------------------

/// @brief Converts a channel ID string into a channel ID value.
#define MDF_BLOCK_ID_STR_TO_VALUE(c0, c1, c2, c3) ((c0) | (c1) << 8 | (c2) << 16 | (c3) << 24)

/// @brief The file identification string of a finalized MDF file. Note MDF files generated with this interface are likely not
/// finalized, I just added it here for completeness.
#define MDF_FILE_IDENTIFICATION_FINALIZED {'M', 'D', 'F', ' ', ' ', ' ', ' ', ' '}

/// @brief The file identification string of an unfinalized MDF file.
#define MDF_FILE_IDENTIFICATION_UNFINALIZED	{'U', 'n', 'F', 'i', 'n', 'M', 'F', ' '}

/// @brief The version string of a version 4.11 MDF file.
#define MDF_VERSION_STRING_V4_11 {'4', '.', '1', '1', ' ', ' ', ' ', ' '}

/// @brief The expected contents of the @c mdfFileIdBlock_t.data field. Note, I do not know what the meaning of these bytes,
/// just that they need be included in the block.
#define MDF_FILE_ID_BLOCK_DATA																								\
{																															\
	0x00, 0x00, 0x00, 0x00, 0x9B, 0x01, 0x00, 0x00,																			\
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,																			\
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,																			\
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,																			\
	0x00, 0x00, 0x00, 0x00, 0x25, 0x00, 0x00, 0x00,																			\
}

/// @brief Datatype representing the MDF file ID block, a unique MDF block occurring only once per file.
typedef struct
{
	/// @brief The file identification string. Indicates the whether the file is finalized or not. Note this string is not
	/// null-terminated.
	char fileIdentification [8];

	/// @brief The file's version string. Indicates the version of the MDF standard this file is compliant to. Should always
	/// be @c MDF_VERSION_STRING_V4_11 . Note this string is not null-terminated.
	char versionString [8];

	/// @brief The program identification string. Indicates what software generated the MDF file. Note this string need not be
	/// null-terminated, but is highly recommended to be in order to avoid programming errors.
	char programIdentification [8];
	uint8_t data [40];
} mdfFileIdBlock_t;

// MDF Blocks -----------------------------------------------------------------------------------------------------------------

/// @brief Datatype representing an MDF block.
typedef struct
{
	/// @brief The file address of the header of the block.
	uint64_t addr;

	/// @brief The block's header.
	struct
	{
		union
		{
			/// @brief The ID of the block.
			uint64_t blockId;

			/// @brief The ID of the block, as a string.
			char blockIdString [8];
		};

		/// @brief The length of the block, including the header, in bytes.
		uint64_t blockLength;

		/// @brief The number of addresses in the block's link list.
		uint64_t linkCount;
	} header;

	/// @brief The block's link list. This is an array of file addresses. The number of elements is given by
	/// @c header.linkCount .
	uint64_t* linkList;

	/// @brief The block's data section. The contents and interpretation of this are specific to the block type (indicated by
	/// @c header.blockId ). The size of this array is given by the @c mdfBlockDataSectionSize function.
	void* dataSection;
} mdfBlock_t;

/**
 * @brief Initializes and allocates an MDF block.
 * @param block The block to initialize. Note the block must be deallocated using @c mdfBlockDealloc .
 * @param blockId The ID to provide the block.
 * @param linkCount The number of links in the link list of the block.
 * @param dataSectionSize The size to allocate for the block's data section.
 * @return 0 if successful, the error code otherwise.
 */
int mdfBlockInit (mdfBlock_t* block, uint64_t blockId, uint64_t linkCount, uint64_t dataSectionSize);

/**
 * @brief Allocates memory for a block based on the contents of its header.
 * @param block The block to initialize. Note the block must be deallocated using @c mdfBlockDealloc .
 * @return 0 if successful, the error code otherwise.
 */
int mdfBlockInitHeader (mdfBlock_t* block);

/**
 * @brief Deallocates a block initialized by @c mdfBlockInit or @c mdfBlockInitHeader .
 * @param block The block to deallocate.
 */
void mdfBlockDealloc (mdfBlock_t* block);

/// @return The size of a block's data section.
static inline uint64_t mdfBlockDataSectionSize (mdfBlock_t* block)
{
	return block->header.blockLength - sizeof (block->header) - block->header.linkCount * sizeof (uint64_t);
}

#endif // MDF_BLOCK_H
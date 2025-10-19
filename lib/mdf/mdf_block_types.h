#ifndef MDF_BLOCK_TYPES_H
#define MDF_BLOCK_TYPES_H

// MDF Block Type Definitions -------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2025.10.09
//
// Description: Definitions of specific types of MDF blocks.
//
// References:
// - https://www.asam.net/standards/detail/mdf/wiki/
// - https://asammdf.readthedocs.io/en/latest/v4blocks.html

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "mdf_block.h"

// ##HD - Header Block --------------------------------------------------------------------------------------------------------

/// @brief The block ID of a header block.
#define MDF_BLOCK_ID_HD MDF_BLOCK_ID_STR_TO_VALUE ('#', '#', 'H', 'D')

/// @brief The data section of a header block.
typedef struct
{
	/// @brief The time, in nanoseconds since the Unix epoch, of the creation of the MDF file.
	uint64_t unixTimeNs;

	/// @brief Reserved, must be set to all 0s.
	uint64_t reserved0 [3];
} mdfHdDataSection_t;

/// @brief The link list of a header block.
typedef struct
{
	/// @brief The address of the first data group in the file.
	uint64_t firstDgAddr;

	/// @brief The address of the first file history block in the file.
	uint64_t firstFhAddr;

	/// @note Uncertain of what this is used for.
	uint64_t channelTreeAddr;

	/// @brief The address of the first attachement in the file.
	uint64_t firstAttachmentAddr;

	/// @note Uncertain of what this is used for.
	uint64_t firstEventAddr;

	/// @brief The address of the comment block.
	uint64_t commentAddr;
} mdfHdLinkList_t;

/**
 * @brief Initializes a header block.
 * @param block The block to initialize. Must be deallocated using @c mdfBlockDealloc .
 * @param dataSection The data section to give the block.
 * @param linkList The link list to give the block.
 * @return 0 if successful, the error code otherwise.
 */
int mdfHdBlockInit (mdfBlock_t* block, mdfHdDataSection_t* dataSection, mdfHdLinkList_t* linkList);

/// @return The link list of the block.
static inline mdfHdLinkList_t* mdfHdBlockLinkList (mdfBlock_t* block) { return (mdfHdLinkList_t*) block->linkList; }

// ##CN - Channel Block -------------------------------------------------------------------------------------------------------

/// @brief The block ID of a channel block.
#define MDF_BLOCK_ID_CN MDF_BLOCK_ID_STR_TO_VALUE ('#', '#', 'C', 'N')

/// @brief Channel type indicating a direct value.
#define MDF_CHANNEL_TYPE_VALUE 0x00

/// @note Uncertain on what this means.
#define MDF_CHANNEL_TYPE_VLSD 0x01

/// @note Uncertain on what this means.
#define MDF_SYNC_TYPE_NONE 0x00

/// @brief Channel data type indicating an unsigned integer encoded in the intel (little endian) format.
#define MDF_DATA_TYPE_UNSIGNED_INTEL 0x00

/// @brief Channel data type indicating an array of bytes.
#define MDF_DATA_TYPE_BYTE_ARRAY 0x0A

/// @brief Channel flags for no effect.
#define MDF_CN_FLAGS_NONE 0x0000

/// @brief Channel flags indicating a bus event.
#define MDF_CN_FLAGS_BUS_EVENT 0x0400

/// @brief The data section of a channel block.
typedef struct
{
	/// @brief The type of this channel.
	/// @note Only @c MDF_CHANNEL_TYPE_VALUE is supported.
	uint8_t channelType;

	/// @note Only @c MDF_SYNC_TYPE_NONE is supported.
	uint8_t syncType;

	/// @brief The data type of the channel.
	uint8_t dataType;

	/// @brief The bit offset of the start of the channel's data. Must be less than 8.
	uint8_t bitOffset;

	/// @brief The byte offset of the start of the channel's data.
	uint8_t byteOffset;

	/// @brief Reserved, must be all 0s.
	uint8_t reserved0 [3];

	/// @brief The length of the channel's data, in bits.
	uint8_t bitLength;

	/// @brief Reserved, must be all 0s.
	uint8_t reserved1 [3];

	/// @brief The channel's flags.
	uint16_t flags;

	/// @brief Reserved, must be all 0s.
	uint8_t reserved2 [58];
} mdfCnDataSection_t;

/// @brief The link list of a channel block.
typedef struct
{
	/// @brief The address of the next channel in the list.
	uint64_t nextCnAddr;

	/// @brief The address of the first component channel forming this channel.
	uint64_t componentAddr;

	/// @brief The address of the text block containing the name of the channel.
	uint64_t nameAddr;

	/// @brief The address of the source of the channel.
	uint64_t sourceAddr;

	/// @brief The address of the channel conversion block, if present.
	uint64_t conversionAddr;

	/// @note Uncertain on what this means.
	uint64_t dataBlockAddr;

	/// @brief The address of the text block containing the unit of the channel.
	uint64_t unitAddr;

	/// @brief The address of the comment.
	uint64_t commentAddr;
} mdfCnLinkList_t;

/**
 * @brief Initializes a markdown block.
 * @param block The block to initialize. Must be deallocated using @c mdfBlockDealloc .
 * @param dataSection The data section to give the block.
 * @param linkList The link list to give the block.
 * @return 0 if successful, the error code otherwise.
 */
int mdfCnBlockInit (mdfBlock_t* block, mdfCnDataSection_t* dataSection, mdfCnLinkList_t* linkList);

/// @return The link list of the block.
static inline mdfCnLinkList_t* mdfCnBlockLinkList (mdfBlock_t* block) { return (mdfCnLinkList_t*) block->linkList; }

// ##CG - Channel Group Block -------------------------------------------------------------------------------------------------

/// @brief The block ID of a channel group block.
#define MDF_BLOCK_ID_CG MDF_BLOCK_ID_STR_TO_VALUE ('#', '#', 'C', 'G')

#define MDF_CG_FLAGS_NONE				0x00
#define MDF_CG_FLAGS_BUS_EVENT			0x02
#define MDF_CG_FLAGS_PLAIN_BUS_EVENT	0x04

/// @brief The data section of a channel group block.
typedef struct
{
	/// @brief The ID used to identify the group's records.
	uint8_t recordId;

	/// @brief Reserved, must be all 0s.
	uint8_t reserved0 [15];

	/// @brief The group's flags.
	uint8_t flags;

	/// @brief Reserved, must be all 0s.
	uint8_t reserved1 [1];

	/// @brief The character used to separate a channel's name from its component channels' names. Ex:
	/// "parentChannel.componentChannel" uses the '.' separator.
	uint8_t pathSeparator;

	/// @brief Reserved, must be all 0s.
	uint8_t reserved2 [5];

	/// @brief The length of the group's record, in bytes.
	uint8_t byteLength;

	/// @brief Reserved, must be all 0s.
	uint8_t reserved3 [7];
} mdfCgDataSection_t;

/// @brief The link list of a channel group block.
typedef struct
{
	/// @brief The address of the next channel group block in the list.
	uint64_t nextCgAddr;

	/// @brief The address of the first channel in the group's list.
	uint64_t firstCnAddr;

	/// @brief The address of the text block indicating the name of the group.
	uint64_t acquisitionNameAddr;

	/// @brief The address of the source information block for the group.
	uint64_t acquisitionSourceAddr;

	/// @note Uncertain on what this means.
	uint64_t firstSampleReductionAddr;

	/// @brief The address of the comment.
	uint64_t commentAddr;
} mdfCgLinkList_t;

/**
 * @brief Initializes a channel group block.
 * @param block The block to initialize. Must be deallocated using @c mdfBlockDealloc .
 * @param dataSection The data section to give the block.
 * @param linkList The link list to give the block.
 * @return 0 if successful, the error code otherwise.
 */
int mdfCgBlockInit (mdfBlock_t* block, mdfCgDataSection_t* dataSection, mdfCgLinkList_t* linkList);

/// @return The link list of the block.
static inline mdfCgLinkList_t* mdfCgBlockLinkList (mdfBlock_t* block) { return (mdfCgLinkList_t*) block->linkList; }

// ##CC - Channel Conversion Block --------------------------------------------------------------------------------------------

/// @brief The block ID of a channel conversion block.
#define MDF_BLOCK_ID_CC MDF_BLOCK_ID_STR_TO_VALUE ('#', '#', 'C', 'C')

/// @brief Conversion type indicating a linear transfer function.
#define MDF_CC_CONVERSION_TYPE_LINEAR 0x01

/// @brief Default conversion flags.
#define MDF_CC_FLAGS_NONE 0x0000

/// @brief The data section of a channel conversion block. Linear conversions implement the below transfer function:
/// y = a * x + b.
typedef struct
{
	/// @brief The type of the conversion block.
	/// @note Only @c MDF_CC_CONVERSION_TYPE_LINEAR is supported.
	uint8_t conversionType;

	/// @note Uncertain what this means.
	uint8_t precision;

	/// @brief The flags of the channel conversion.
	uint16_t flags;

	/// @note Uncertain what this means.
	uint16_t referenceParameterNumber;

	/// @note Uncertain what this means.
	uint16_t valueParameterNumber;

	/// @brief The minimum physical value.
	double minPhysicalValue;

	/// @brief The maximum physical value.
	double maxPhysicalValue;

	/// @brief The offset to apply.
	double b;

	/// @brief The scale factor to apply.
	double a;
} mdfCcDataSection_t;

/// @brief The link list of a channel conversion block.
typedef struct
{
	/// @brief The address of the text block containing the name of the conversion.
	uint64_t nameAddr;

	/// @brief The address of the text block containing the unit of the post-conversion value.
	uint64_t unitAddr;

	/// @brief The address of the comment.
	uint64_t commentAddr;

	/// @brief The address of the conversion channel implementing the inversion of this conversion.
	uint64_t inverseConversionAddr;
} mdfCcLinkList_t;

/**
 * @brief Initializes a conversion channel block.
 * @param block The block to initialize. Must be deallocated using @c mdfBlockDealloc .
 * @param dataSection The data section to give the block.
 * @param linkList The link list to give the block.
 * @return 0 if successful, the error code otherwise.
 */
int mdfCcBlockInit (mdfBlock_t* block, mdfCcDataSection_t* dataSection, mdfCcLinkList_t* linkList);

/// @return The link list of the block.
static inline mdfCcLinkList_t* mdfCcBlockLinkList (mdfBlock_t* block) { return (mdfCcLinkList_t*) block->linkList; }

// ##DG - Data Group Block ----------------------------------------------------------------------------------------------------

/// @brief The block ID of a data group block.
#define MDF_BLOCK_ID_DG MDF_BLOCK_ID_STR_TO_VALUE ('#', '#', 'D', 'G')

/// @brief The data section of a data group block.
typedef struct
{
	/// @brief The address of the next data group in the list.
	uint64_t nextDgAddr;

	/// @brief The address of the first channel group in the list.
	uint64_t firstCgAddr;

	/// @brief The address of the data group's data block.
	uint64_t dataBlockAddr;

	/// @brief The address of the comment.
	uint64_t commentAddr;
} mdfDgLinkList_t;

/**
 * @brief Initializes a data group block.
 * @param block The block to initialize. Must be deallocated using @c mdfBlockDealloc .
 * @param linkList The link list to give the block.
 * @return 0 if successful, the error code otherwise.
 */
int mdfDgBlockInit (mdfBlock_t* block, mdfDgLinkList_t* linkList);

/// @return The link list of the block.
static inline mdfDgLinkList_t* mdfDgBlockLinkList (mdfBlock_t* block) { return (mdfDgLinkList_t*) block->linkList; }

// ##DT - Data Block ----------------------------------------------------------------------------------------------------------

/// @brief The block ID of a data block.
#define MDF_BLOCK_ID_DT MDF_BLOCK_ID_STR_TO_VALUE ('#', '#', 'D', 'T')

/**
 * @brief Initializes a conversion channel block.
 * @param block The block to initialize. Must be deallocated using @c mdfBlockDealloc .
 * @return 0 if successful, the error code otherwise.
 */
int mdfDtBlockInit (mdfBlock_t* block);

// ##SI - Source Information Block --------------------------------------------------------------------------------------------

/// @brief The block ID of a source information block.
#define MDF_BLOCK_ID_SI MDF_BLOCK_ID_STR_TO_VALUE ('#', '#', 'S', 'I')

#define MDF_SOURCE_TYPE_OTHER	0x00
#define MDF_SOURCE_TYPE_ECU		0x01
#define MDF_SOURCE_TYPE_BUS		0x02

#define MDF_BUS_TYPE_NONE		0x00
#define MDF_BUS_TYPE_OTHER		0x01
#define MDF_BUS_TYPE_CAN		0x02

/// @brief The data section of a source information block.
typedef struct
{
	/// @brief Indicates the source of the data.
	uint8_t sourceType;

	/// @brief Indicates the type of the bus, if the @c sourceType is @c MDF_SOURCE_TYPE_BUS .
	uint8_t busType;

	/// @brief Reserved, must be all 0s.
	uint8_t reserved0 [6];
} mdfSiDataSection_t;

/// @brief The link list of a source information block.
typedef struct
{
	/// @brief The address of the text block containing the source's name.
	uint64_t nameAddr;

	/// @brief The address of the text block containing the source path's name.
	uint64_t pathAddr;

	/// @brief The address of the comment.
	uint64_t commentAddr;
} mdfSiLinkList_t;

/**
 * @brief Initializes a source information block.
 * @param block The block to initialize. Must be deallocated using @c mdfBlockDealloc .
 * @param dataSection The data section to give the block.
 * @param linkList The link list to give the block.
 * @return 0 if successful, the error code otherwise.
 */
int mdfSiBlockInit (mdfBlock_t* block, mdfSiDataSection_t* dataSection, mdfSiLinkList_t* linkList);

/// @return The link list of the block.
static inline mdfSiLinkList_t* mdfSiBlockLinkList (mdfBlock_t* block) { return (mdfSiLinkList_t*) block->linkList; }

// ##FH - File History Block --------------------------------------------------------------------------------------------------

/// @brief The block ID of a file history block.
#define MDF_BLOCK_ID_FH MDF_BLOCK_ID_STR_TO_VALUE ('#', '#', 'F', 'H')

/// @brief The data section of a file history block.
typedef struct
{
	/// @brief The time, in nanoseconds since the Unix epoch, of the event's occurrence.
	uint64_t unixTimeNs;

	/// @brief Reserved, must be all 0s.
	uint64_t reserved0;
} mdfFhDataSection_t;

/// @brief The link list of a file history block.
typedef struct
{
	/// @brief The address of the next file history block in the list.
	uint64_t nextFhAddr;

	/// @brief The address of the comment.
	uint64_t commentAddr;
} mdfFhLinkList_t;

/**
 * @brief Initializes a file history block.
 * @param block The block to initialize. Must be deallocated using @c mdfBlockDealloc .
 * @param dataSection The data section to give the block.
 * @param linkList The link list to give the block.
 * @return 0 if successful, the error code otherwise.
 */
int mdfFhBlockInit (mdfBlock_t* block, mdfFhDataSection_t* dataSection, mdfFhLinkList_t* linkList);

/// @return The link list of the block.
static inline mdfFhLinkList_t* mdfFhBlockLinkList (mdfBlock_t* block) { return (mdfFhLinkList_t*) block->linkList; }

// ##TX - Text Block ----------------------------------------------------------------------------------------------------------

/// @brief The block ID of a text block.
#define MDF_BLOCK_ID_TX MDF_BLOCK_ID_STR_TO_VALUE ('#', '#', 'T', 'X')

/**
 * @brief Initializes a text block.
 * @param block The block to initialize. Must be deallocated using @c mdfBlockDealloc .
 * @param text The text to put in the block.
 * @return 0 if successful, the error code otherwise.
 */
int mdfTxBlockInit (mdfBlock_t* block, const char* text);

// ##MD - Markdown Block ------------------------------------------------------------------------------------------------------

/// @brief The block ID of a markdown block.
#define MDF_BLOCK_ID_MD MDF_BLOCK_ID_STR_TO_VALUE ('#', '#', 'M', 'D')

/**
 * @brief Initializes a markdown block.
 * @param block The block to initialize. Must be deallocated using @c mdfBlockDealloc .
 * @param xml The XML to put in the block.
 * @return 0 if successful, the error code otherwise.
 */
int mdfMdBlockInit (mdfBlock_t* block, const char* xml);

#endif // MDF_BLOCK_TYPES_H
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

// General  -------------------------------------------------------------------------------------------------------------------

/**
 * @brief Writes the file ID block to an MDF file.
 * @param mdf The file to write to.
 * @param fileIdBlock The file ID block to write.
 * @return 0 if successful, the error code otherwise.
 */
int mdfWriteFileIdBlock (FILE* mdf, mdfFileIdBlock_t* fileIdBlock);

/**
 * @brief Writes a block to an MDF file.
 * @param mdf The file to write to.
 * @param block The block to write.
 * @return The address of the block if successful, 0 otherwise.
 */
uint64_t mdfBlockWrite (FILE* mdf, mdfBlock_t* block);

/**
 * @brief Re-writes a block's link list to an MDF file. This is used to when the block's complete link list isn't known at the
 * time of writing it.
 * @param mdf The file to write to.
 * @param block The block to rewrite.
 * @return 0 if successful, the error code otherwise.
 */
int mdfRewriteBlockLinkList (FILE* mdf, mdfBlock_t* block);

/**
 * @brief Re-writes a block's data section to an MDF file. This is used when a block's complete data section isn't known at the
 * time of writing it.
 * @param mdf The file to write to.
 * @param block The block to re-write.
 * @return 0 if successful, the error code otherwise.
 */
int mdfRewriteBlockDataSection (FILE* mdf, mdfBlock_t* block);

// Block Types ----------------------------------------------------------------------------------------------------------------

/**
 * @brief Creates and writes a channel block to an MDF file.
 * @param mdf The file to write to.
 * @param dataSection The data section to write.
 * @param linkList The link list to write.
 * @return The address of the block if successful, 0 otherwise.
 */
uint64_t mdfCnBlockWrite (FILE* mdf, mdfCnDataSection_t* dataSection, mdfCnLinkList_t* linkList);

/**
 * @brief Creates and writes a channel group block to an MDF file.
 * @param mdf The file to write to.
 * @param dataSection The data section to write.
 * @param linkList The link list to write.
 * @return The address of the block if successful, 0 otherwise.
 */
uint64_t mdfCgBlockWrite (FILE* mdf, mdfCgDataSection_t* dataSection, mdfCgLinkList_t* linkList);

/**
 * @brief Creates and writes a channel conversion to an MDF file.
 * @param mdf The file to write to.
 * @param dataSection The data section to write.
 * @param linkList The link list to write.
 * @return The address of the block if successful, 0 otherwise.
 */
uint64_t mdfCcBlockWrite (FILE* mdf, mdfCcDataSection_t* dataSection, mdfCcLinkList_t* linkList);

/**
 * @brief Creates and writes a data group block to an MDF file.
 * @param mdf The file to write to.
 * @param linkList The link list to write.
 * @return The address of the block if successful, 0 otherwise.
 */
uint64_t mdfDgBlockWrite (FILE* mdf, mdfDgLinkList_t* linkList);

/**
 * @brief Creates and writes a data block to an MDF file.
 * @param mdf The file to write to.
 * @return The address of the block if successful, 0 otherwise.
 */
uint64_t mdfDtBlockWrite (FILE* mdf);

/**
 * @brief Creates and writes a source information block to an MDF file.
 * @param mdf The file to write to.
 * @param dataSection The data section to write.
 * @param linkList The link list to write.
 * @return The address of the block if successful, 0 otherwise.
 */
uint64_t mdfSiBlockWrite (FILE* mdf, mdfSiDataSection_t* dataSection, mdfSiLinkList_t* linkList);

/**
 * @brief Creates and writes a file history block to an MDF file.
 * @param mdf The file to write to.
 * @param dataSection The data section to write.
 * @param linkList The link list to write.
 * @return The address of the block if successful, 0 otherwise.
 */
uint64_t mdfFhBlockWrite (FILE* mdf, mdfFhDataSection_t* dataSection, mdfFhLinkList_t* linkList);

/**
 * @brief Creates and writes a channel group block to an MDF file.
 * @param mdf The file to write to.
 * @param text The text to write.
 * @return The address of the block if successful, 0 otherwise.
 */
uint64_t mdfTxBlockWrite (FILE* mdf, const char* text);

/**
 * @brief Creates and writes a markdown block to an MDF file.
 * @param mdf The file to write to.
 * @param xml The XML to write.
 * @return The address of the block if successful, 0 otherwise.
 */
uint64_t mdfMdBlockWrite (FILE* mdf, const char* xml);

#endif // MDF_WRITER_H
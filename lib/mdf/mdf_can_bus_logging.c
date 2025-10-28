// Header
#include "mdf_can_bus_logging.h"

// Includes
#include "mdf_writer.h"

// C Standard Library
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#define CAN_DATA_FRAME_BIT_LENGTH 104
#define CAN_DATA_FRAME_BYTE_OFFSET 6

#define BIT_LENGTH_TO_BYTE_LENGTH(bitLength) (((bitLength) + 7) / 8)
#define BIT_LENGTH_TO_BIT_MASK(bitLength) ((1 << (bitLength)) - 1)

#define TIMESTAMP_BYTE_OFFSET	0
#define TIMESTAMP_BIT_LENGTH	48

#define ID_BYTE_OFFSET			6
#define ID_BIT_LENGTH			29

#define IDE_BYTE_OFFSET			9
#define IDE_BIT_OFFSET			5
#define IDE_BIT_LENGTH			1

#define BUS_CHANNEL_BYTE_OFFSET	9
#define BUS_CHANNEL_BIT_OFFSET	6
#define BUS_CHANNEL_BIT_LENGTH	2

#define DLC_BYTE_OFFSET			10
#define DLC_BIT_OFFSET			0
#define DLC_BIT_LENGTH			4

#define DATA_BYTES_BYTE_OFFSET	11
#define DATA_BYTES_BIT_LENGTH	64

/**
 * @brief Writes the header of a CAN bus MDF log.
 * @param mdf The MDF file to write to. Note this file cannot have anything else written to it before this call.
 * @param programId The ID of the program generating this file. Note, strings longer than 7 characters will be truncated.
 * @param unixTime The Unix timestamp at which this file was created.
 * @return The header block (##HD) of the MDF file. Note this is dynamically allocated, and must be deallocated using
 * @c mdfBlockDealloc followed by @c free .
 */
static mdfBlock_t* writeHeader (FILE* mdf, const char* programId, time_t unixTime)
{
	// All MDF files start with the file ID block
	mdfFileIdBlock_t fileIdBlock =
	{
		.fileIdentification		= MDF_FILE_IDENTIFICATION_UNFINALIZED,
		.versionString			= MDF_VERSION_STRING_V4_11,
		.data					= MDF_FILE_ID_BLOCK_DATA
	};

	// Copy the program ID into the file ID block.
	strncpy (fileIdBlock.programIdentification, programId, sizeof (fileIdBlock.programIdentification));
	fileIdBlock.programIdentification [sizeof (fileIdBlock.programIdentification) - 1] = '\0';

	// Write the file ID block
	if (mdfWriteFileIdBlock (mdf, &fileIdBlock) != 0)
		return NULL;

	// The block immediately after the file ID must be the header block. This is the root of the MDF file's hierarchy.
	mdfBlock_t* block = malloc (sizeof (mdfBlock_t));
	if (mdfHdBlockInit (block,
		&(mdfHdDataSection_t)
		{
			.unixTimeNs = unixTime * 1e9 // File timestamp
		},
		&(mdfHdLinkList_t)
		{
			// Link list is populated later, as we don't know the addresses of any of these yet.
		}
	) != 0)
	{
		free (block);
		return NULL;
	}

	// Write the header block
	if (mdfBlockWrite (mdf, block) == 0)
	{
		mdfBlockDealloc (block);
		free (block);
		return NULL;
	}

	return block;
}

static uint64_t writeAcquisitionSource (FILE* mdf)
{
	// Write the name / path text block.
	uint64_t nameAddr = mdfTxBlockWrite (mdf, "CAN");
	if (nameAddr == 0)
		return 0;

	// TODO(Barach): Variadic args for this?
	// Write the comment block.
	uint64_t commentAddr = mdfMdBlockWrite (mdf,
			"<SIcomment>\n"
			"<TX>\n"
			"    CAN\n"
			"</TX>\n"
			"<bus name=\"CAN\"/>\n"
			"    <common_properties>\n"
			"        <tree name=\"ASAM Measurement Environment\">\n"
			"            <tree name=\"node\">\n"
			"                <e name=\"type\">Device</e>\n"
			"                <e name=\"software configuration\">01.04.02</e>\n"
			"                <e name=\"hardware version\">00.02</e>\n"
			"                <e name=\"serial number\">846DD296</e>\n"
			"            </tree>\n"
			"        </tree>\n"
			"        <tree name=\"Bus Information\">\n"
			"            <e name=\"CAN1 Bit-rate\" unit=\"Hz\">1000000</e>\n"
			"            <e name=\"CAN2 Bit-rate\" unit=\"Hz\">1000000</e>\n"
			"        </tree>\n"
			"    </common_properties>\n"
			"</SIcomment>");
	if (commentAddr == 0)
		return 0;

	// Write the acquisition source block.
	return mdfSiBlockWrite (mdf,
		&(mdfSiDataSection_t)
		{
			.sourceType	= MDF_SOURCE_TYPE_BUS,
			.busType	= MDF_BUS_TYPE_CAN
		},
		&(mdfSiLinkList_t)
		{
			.nameAddr		= nameAddr,
			.pathAddr		= nameAddr,
			.commentAddr	= commentAddr
		});
}

static uint64_t writeDataFrameCg (FILE* mdf, uint64_t nextCgAddr, uint64_t acquisitionSourceAddr, uint64_t timestampCcAddr)
{
	// Component Channels -----------------------------------------------------------------------------------------------------

	// Write the name block
	uint64_t dataBytesNameAddr = mdfTxBlockWrite (mdf, "CAN_DataFrame.DataBytes");
	if (dataBytesNameAddr == 0)
		return 0;

	// Write the channel block
	uint64_t dataBytesCnAddr = mdfCnBlockWrite (mdf,
		&(mdfCnDataSection_t)
		{
			.channelType	= MDF_CHANNEL_TYPE_VALUE,
			.syncType		= MDF_SYNC_TYPE_NONE,
			.dataType		= MDF_DATA_TYPE_BYTE_ARRAY,
			.bitOffset		= 0,
			.byteOffset		= DATA_BYTES_BYTE_OFFSET,
			.bitLength		= DATA_BYTES_BIT_LENGTH,
			.flags			= MDF_CN_FLAGS_BUS_EVENT
		},
		&(mdfCnLinkList_t)
		{
			.nameAddr	= dataBytesNameAddr,
			.nextCnAddr	= 0
		});
	if (dataBytesCnAddr == 0)
		return 0;

	uint64_t dirNameAddr = mdfTxBlockWrite (mdf, "CAN_DataFrame.Dir");
	if (dirNameAddr == 0)
		return 0;

	uint64_t dirCnAddr = mdfCnBlockWrite (mdf,
		&(mdfCnDataSection_t)
		{
			.channelType	= MDF_CHANNEL_TYPE_VALUE,
			.syncType		= MDF_SYNC_TYPE_NONE,
			.dataType		= MDF_DATA_TYPE_UNSIGNED_INTEL,
			.bitOffset		= 4,
			.byteOffset		= 10,
			.bitLength		= 1,
			.flags			= MDF_CN_FLAGS_BUS_EVENT
		},
		&(mdfCnLinkList_t)
		{
			.nameAddr	= dirNameAddr,
			.nextCnAddr	= dataBytesCnAddr
		});
	if (dirCnAddr == 0)
		return 0;

	uint64_t dataLengthNameAddr = mdfTxBlockWrite (mdf, "CAN_DataFrame.DataLength");
	if (dataLengthNameAddr == 0)
		return 0;

	uint64_t dataLengthCnAddr = mdfCnBlockWrite (mdf,
		&(mdfCnDataSection_t)
		{
			.channelType	= MDF_CHANNEL_TYPE_VALUE,
			.syncType		= MDF_SYNC_TYPE_NONE,
			.dataType		= MDF_DATA_TYPE_UNSIGNED_INTEL,
			.bitOffset		= DLC_BIT_OFFSET,
			.byteOffset		= DLC_BYTE_OFFSET,
			.bitLength		= DLC_BIT_LENGTH,
			.flags			= MDF_CN_FLAGS_BUS_EVENT
		},
		&(mdfCnLinkList_t)
		{
			.nameAddr	= dataLengthNameAddr,
			.nextCnAddr	= dirCnAddr
		});
	if (dataLengthCnAddr == 0)
		return 0;

	uint64_t dlcNameAddr = mdfTxBlockWrite (mdf, "CAN_DataFrame.DLC");
	if (dlcNameAddr == 0)
		return 0;

	uint64_t dlcCnAddr = mdfCnBlockWrite (mdf,
		&(mdfCnDataSection_t)
		{
			.channelType	= MDF_CHANNEL_TYPE_VALUE,
			.syncType		= MDF_SYNC_TYPE_NONE,
			.dataType		= MDF_DATA_TYPE_UNSIGNED_INTEL,
			.bitOffset		= DLC_BIT_OFFSET,
			.byteOffset		= DLC_BYTE_OFFSET,
			.bitLength		= DLC_BIT_LENGTH,
			.flags			= MDF_CN_FLAGS_BUS_EVENT
		},
		&(mdfCnLinkList_t)
		{
			.nameAddr	= dlcNameAddr,
			.nextCnAddr	= dataLengthCnAddr
		});
	if (dlcCnAddr == 0)
		return 0;

	uint64_t busChannelNameAddr = mdfTxBlockWrite (mdf, "CAN_DataFrame.BusChannel");
	if (busChannelNameAddr == 0)
		return 0;

	uint64_t busChannelCnAddr = mdfCnBlockWrite (mdf,
		&(mdfCnDataSection_t)
		{
			.channelType	= MDF_CHANNEL_TYPE_VALUE,
			.syncType		= MDF_SYNC_TYPE_NONE,
			.dataType		= MDF_DATA_TYPE_UNSIGNED_INTEL,
			.bitOffset		= BUS_CHANNEL_BIT_OFFSET,
			.byteOffset		= BUS_CHANNEL_BYTE_OFFSET,
			.bitLength		= BUS_CHANNEL_BIT_LENGTH,
			.flags			= MDF_CN_FLAGS_BUS_EVENT
		},
		&(mdfCnLinkList_t)
		{
			.nameAddr	= busChannelNameAddr,
			.nextCnAddr	= dlcCnAddr
		});
	if (busChannelCnAddr == 0)
		return 0;

	uint64_t ideNameAddr = mdfTxBlockWrite (mdf, "CAN_DataFrame.IDE");
	if (ideNameAddr == 0)
		return 0;

	uint64_t ideCnAddr = mdfCnBlockWrite (mdf,
		&(mdfCnDataSection_t)
		{
			.channelType	= MDF_CHANNEL_TYPE_VALUE,
			.syncType		= MDF_SYNC_TYPE_NONE,
			.dataType		= MDF_DATA_TYPE_UNSIGNED_INTEL,
			.bitOffset		= IDE_BIT_OFFSET,
			.byteOffset		= IDE_BYTE_OFFSET,
			.bitLength		= IDE_BIT_LENGTH,
			.flags			= MDF_CN_FLAGS_BUS_EVENT
		},
		&(mdfCnLinkList_t)
		{
			.nameAddr	= ideNameAddr,
			.nextCnAddr	= busChannelCnAddr
		});
	if (ideCnAddr == 0)
		return 0;

	uint64_t idNameAddr = mdfTxBlockWrite (mdf, "CAN_DataFrame.ID");
	if (idNameAddr == 0)
		return 0;

	uint64_t idCnAddr = mdfCnBlockWrite (mdf,
		&(mdfCnDataSection_t)
		{
			.channelType	= MDF_CHANNEL_TYPE_VALUE,
			.syncType		= MDF_SYNC_TYPE_NONE,
			.dataType		= MDF_DATA_TYPE_UNSIGNED_INTEL,
			.bitOffset		= 0,
			.byteOffset		= ID_BYTE_OFFSET,
			.bitLength		= ID_BIT_LENGTH,
			.flags			= MDF_CN_FLAGS_BUS_EVENT
		},
		&(mdfCnLinkList_t)
		{
			.nameAddr	= idNameAddr,
			.nextCnAddr	= ideCnAddr
		});
	if (idCnAddr == 0)
		return 0;

	// Channels ---------------------------------------------------------------------------------------------------------------

	uint64_t dataFrameNameAddr = mdfTxBlockWrite (mdf, "CAN_DataFrame");
	if (dataFrameNameAddr == 0)
		return 0;

	uint64_t dataFrameCnAddr = mdfCnBlockWrite (mdf,
		&(mdfCnDataSection_t)
		{
			.channelType	= MDF_CHANNEL_TYPE_VALUE,
			.syncType		= MDF_SYNC_TYPE_NONE,
			.dataType		= MDF_DATA_TYPE_BYTE_ARRAY,
			.bitOffset		= 0,
			.byteOffset		= CAN_DATA_FRAME_BYTE_OFFSET,
			.bitLength		= CAN_DATA_FRAME_BIT_LENGTH,
			.flags			= MDF_CN_FLAGS_BUS_EVENT
		},
		&(mdfCnLinkList_t)
		{
			.nameAddr		= dataFrameNameAddr,
			.componentAddr	= idCnAddr,
			.nextCnAddr		= 0,
		});

	uint64_t timestampNameAddr = mdfTxBlockWrite (mdf, "Timestamp");
	if (timestampNameAddr == 0)
		return 0;

	uint64_t timestampCnAddr = mdfCnBlockWrite (mdf,
		&(mdfCnDataSection_t)
		{
			.channelType	= MDF_CHANNEL_TYPE_MASTER,
			.syncType		= MDF_SYNC_TYPE_TIME,
			.dataType		= MDF_DATA_TYPE_UNSIGNED_INTEL,
			.bitOffset		= 0,
			.byteOffset		= TIMESTAMP_BYTE_OFFSET,
			.bitLength		= TIMESTAMP_BIT_LENGTH,
			.flags			= MDF_CN_FLAGS_NONE
		},
		&(mdfCnLinkList_t)
		{
			.nameAddr		= timestampNameAddr,
			.nextCnAddr		= dataFrameCnAddr,
			.conversionAddr	= timestampCcAddr
		});
	if (timestampCnAddr == 0)
		return 0;

	// Channel Group ----------------------------------------------------------------------------------------------------------

	return mdfCgBlockWrite (mdf,
		&(mdfCgDataSection_t)
		{
			.recordId		= 0x01,
			.flags			= MDF_CG_FLAGS_BUS_EVENT | MDF_CG_FLAGS_PLAIN_BUS_EVENT,
			.pathSeparator	= '.',
			.byteLength		= 19
		},
		&(mdfCgLinkList_t)
		{
			.nextCgAddr				= nextCgAddr,
			.firstCnAddr			= timestampCnAddr,
			.acquisitionNameAddr	= dataFrameNameAddr,
			.acquisitionSourceAddr	= acquisitionSourceAddr
		});
}

static uint64_t writeTimestampCc (FILE* mdf)
{
	uint64_t timestampUnitAddr = mdfTxBlockWrite (mdf, "s");
	if (timestampUnitAddr == 0)
		return 0;

	return mdfCcBlockWrite (mdf,
		&(mdfCcDataSection_t)
		{
			.conversionType				= MDF_CC_CONVERSION_TYPE_LINEAR,
			.precision					= 0,
			.flags						= MDF_CC_FLAGS_NONE,
			.referenceParameterNumber	= 0,
			.valueParameterNumber		= 2,
			.minPhysicalValue			= 0.0,
			.maxPhysicalValue			= 0.0,
			.b							= 0.0,
			.a							= 1e-6,
		},
		&(mdfCcLinkList_t)
		{
			.unitAddr = timestampUnitAddr
		});
}

static uint64_t writeFileHistory (FILE* mdf, time_t unixTime)
{
	// TODO(Barach): Variadic format specifiers?

	return mdfFhBlockWrite (mdf,
		&(mdfFhDataSection_t)
		{
			.unixTimeNs = unixTime * 1e9
		},
		&(mdfFhLinkList_t)
		{
			.commentAddr = mdfMdBlockWrite (mdf,
				"<FHcomment>\n"
				"	<TX>\n"
				"		Creation and logging of data.\n"
				"	</TX>\n"
				"	<tool_id>CE</tool_id>\n"
				"	<tool_vendor></tool_vendor>\n"
				"	<tool_version>01.04.02</tool_version>\n"
				"</FHcomment>")
		});
}

static uint64_t writeComment (FILE* mdf)
{
	// TODO(Barach): Variadic format specifiers?

	return mdfMdBlockWrite (mdf,
		"<HDcomment>\n"
		"    <TX/>\n"
		"    <common_properties>\n"
		"        <tree name=\"Device Information\">\n"
		"            <e name=\"firmware version\" ro=\"true\">01.04.02</e>\n"
		"            <e name=\"hardware version\" ro=\"true\">00.02</e>\n"
		"            <e name=\"serial number\" ro=\"true\">846DD296</e>\n"
		"            <e name=\"device type\" ro=\"true\">0000001D</e>\n"
		"            <e name=\"storage total\" ro=\"true\">  7746768</e>\n"
		"            <e name=\"storage free\" ro=\"true\">  7746708</e>\n"
		"            <e name=\"config crc32 checksum\" ro=\"true\">5611BDB3</e>\n"
		"        </tree>\n"
		"        <tree name=\"File Information\">\n"
		"            <e name=\"session\" ro=\"true\">     273</e>\n"
		"            <e name=\"split\" ro=\"true\">       1</e>\n"
		"            <e name=\"comment\" ro=\"true\">                              </e>\n"
		"        </tree>\n"
		"    </common_properties>\n"
		"</HDcomment>");
}

static mdfBlock_t* writeDg (FILE* mdf, uint64_t firstCgAddr)
{
	mdfBlock_t* block = malloc (sizeof (mdfBlock_t));
	if (mdfDgBlockInit (block,
		&(mdfDgDataSection_t)
		{
			.recordIdLength = 1
		},
		&(mdfDgLinkList_t)
		{
			.firstCgAddr = firstCgAddr
		}) != 0)
	{
		free (block);
		return NULL;
	}

	if (mdfBlockWrite (mdf, block) == 0)
	{
		mdfBlockDealloc (block);
		free (block);
		return NULL;
	}

	return block;
}

int mdfCanBusLogInit (FILE* mdf, const char* programId, time_t unixTime)
{
	mdfBlock_t* hd = writeHeader (mdf, programId, unixTime);
	if (hd == NULL)
		return errno;

	uint64_t acquisitionSourceAddr = writeAcquisitionSource (mdf);
	if (acquisitionSourceAddr == 0)
		return errno;

	uint64_t timestampCcAddr = writeTimestampCc (mdf);
	if (timestampCcAddr == 0)
		return errno;

	uint64_t dataFrameCgAddr = writeDataFrameCg (mdf, 0, acquisitionSourceAddr, timestampCcAddr);
	if (dataFrameCgAddr == 0)
		return errno;

	uint64_t fileHistoryAddr = writeFileHistory (mdf, unixTime);
	if (fileHistoryAddr == 0)
		return errno;

	uint64_t commentAddr = writeComment (mdf);
	if (commentAddr == 0)
		return errno;

	mdfBlock_t* dg = writeDg (mdf, dataFrameCgAddr);
	if (dg == NULL)
		return errno;

	// Rewrite Link Lists -----------------------------------------------------------------------------------------------------

	uint64_t dtAddr = mdfDtBlockWrite (mdf);
	if (dtAddr == 0)
		return errno;

	mdfDgBlockLinkList (dg)->dataBlockAddr = dtAddr;

	if (mdfRewriteBlockLinkList (mdf, dg) != 0)
		return errno;

	mdfHdBlockLinkList (hd)->firstFhAddr = fileHistoryAddr;
	mdfHdBlockLinkList (hd)->commentAddr = commentAddr;
	mdfHdBlockLinkList (hd)->firstDgAddr = dg->addr;

	if (mdfRewriteBlockLinkList (mdf, hd) != 0)
		return errno;

	mdfBlockDealloc (dg);
	mdfBlockDealloc (hd);

	free (dg);
	free (hd);

	return 0;
}

int mdfCanBusLogWriteDataFrame (FILE* mdf, canFrame_t* frame, uint64_t timestamp, uint8_t busChannel)
{
	uint8_t record [20] = {};

	// Record ID
	record [0] = 0x01;

	// Timestamp
	for (size_t index = 0; index < BIT_LENGTH_TO_BYTE_LENGTH (TIMESTAMP_BIT_LENGTH); ++index)
		record [index + TIMESTAMP_BYTE_OFFSET + 1] |= timestamp >> (index * 8);

	// CAN ID
	for (size_t index = 0; index < BIT_LENGTH_TO_BYTE_LENGTH (ID_BIT_LENGTH); ++index)
		record [index + ID_BYTE_OFFSET + 1] |= frame->id >> (index * 8);

	// IDE
	// TODO(Barach): Hacky workaround.
	bool ide = frame->id >= (1 << 11);
	record [IDE_BYTE_OFFSET + 1] |= (ide & BIT_LENGTH_TO_BIT_MASK (IDE_BIT_LENGTH)) << IDE_BIT_OFFSET;

	// Bus channel
	record [BUS_CHANNEL_BYTE_OFFSET + 1] |=
		(busChannel & BIT_LENGTH_TO_BIT_MASK (BUS_CHANNEL_BIT_LENGTH)) << BUS_CHANNEL_BIT_OFFSET;

	// DLC
	record [DLC_BYTE_OFFSET + 1] |=
		(frame->dlc & BIT_LENGTH_TO_BIT_MASK (DLC_BIT_LENGTH)) << DLC_BIT_OFFSET;

	// Data Bytes
	for (size_t index = 0; index < frame->dlc; ++index)
		record [DATA_BYTES_BYTE_OFFSET + index + 1] |= frame->data [index];

	// Write the record to the file.
	if (fwrite (record, 1, sizeof (record), mdf) != sizeof (record))
		return errno;

	return 0;
}
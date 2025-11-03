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

#define TIMESTAMP_SCALE_FACTOR	1e6
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

static mdfBlock_t* writeHeader (FILE* mdf, const char* programId, time_t timeStart)
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
			.unixTimeNs = timeStart * 1e9 // File timestamp
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

static uint64_t writeAcquisitionSource (FILE* mdf, const char* softwareVersion, const char* hardwareVersion,
	const char* serialNumber, uint32_t channel1Baudrate, uint32_t channel2Baudrate)
{
	// Write the name / path text block.
	uint64_t nameAddr = mdfTxBlockWrite (mdf, "CAN");
	if (nameAddr == 0)
		return 0;

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
			"                <e name=\"software version\">%s</e>\n"
			"                <e name=\"hardware version\">%s</e>\n"
			"                <e name=\"serial number\">%s</e>\n"
			"            </tree>\n"
			"        </tree>\n"
			"        <tree name=\"Bus Information\">\n"
			"            <e name=\"CAN1 Bit-rate\" unit=\"Hz\">%u</e>\n"
			"            <e name=\"CAN2 Bit-rate\" unit=\"Hz\">%u</e>\n"
			"        </tree>\n"
			"    </common_properties>\n"
			"</SIcomment>", softwareVersion, hardwareVersion, serialNumber, channel1Baudrate, channel2Baudrate);
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
			.a							= 1.0f / TIMESTAMP_SCALE_FACTOR,
		},
		&(mdfCcLinkList_t)
		{
			.unitAddr = timestampUnitAddr
		});
}

static uint64_t writeFileHistory (FILE* mdf, time_t timeStart, char* programId, char* softwareVersion)
{
	return mdfFhBlockWrite (mdf,
		&(mdfFhDataSection_t)
		{
			.unixTimeNs = timeStart * 1e9
		},
		&(mdfFhLinkList_t)
		{
			.commentAddr = mdfMdBlockWrite (mdf,
				"<FHcomment>\n"
				"	<TX>\n"
				"		Creation and logging of data.\n"
				"	</TX>\n"
				"	<tool_id>%s</tool_id>\n"
				"	<tool_vendor></tool_vendor>\n"
				"	<tool_version>%s</tool_version>\n"
				"</FHcomment>", programId, softwareVersion)
		});
}

static uint64_t writeComment (FILE* mdf, const char* softwareVersion, const char* hardwareVersion, const char* serialNumber,
	const char* programId, size_t storageSize, size_t storageRemaining, uint32_t sessionNumber, uint32_t splitNumber)
{
	return mdfMdBlockWrite (mdf,
		"<HDcomment>\n"
		"    <TX/>\n"
		"    <common_properties>\n"
		"        <tree name=\"Device Information\">\n"
		"            <e name=\"software version\" ro=\"true\">%s</e>\n"
		"            <e name=\"hardware version\" ro=\"true\">%s</e>\n"
		"            <e name=\"serial number\" ro=\"true\">%s</e>\n"
		"            <e name=\"device type\" ro=\"true\">%s</e>\n"
		"            <e name=\"storage total\" ro=\"true\">%lu</e>\n"
		"            <e name=\"storage free\" ro=\"true\">%lu</e>\n"
		"        </tree>\n"
		"        <tree name=\"File Information\">\n"
		"            <e name=\"session\" ro=\"true\">%u</e>\n"
		"            <e name=\"split\" ro=\"true\">%u</e>\n"
		"            <e name=\"comment\" ro=\"true\"></e>\n"
		"        </tree>\n"
		"    </common_properties>\n"
		"</HDcomment>", softwareVersion, hardwareVersion, serialNumber, programId, storageSize, storageRemaining,
			sessionNumber, splitNumber);
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

int mdfCanBusLogInit (mdfCanBusLog_t* log, const mdfCanBusLogConfig_t* config)
{
	log->config = config;

	log->mdf = fopen (config->filePath, "w");
	if (log->mdf == NULL)
		return errno;

	mdfBlock_t* hd = writeHeader (log->mdf, config->programId, config->timeStart);
	if (hd == NULL)
		return errno;

	uint64_t acquisitionSourceAddr = writeAcquisitionSource (log->mdf, config->softwareVersion, config->hardwareVersion,
		config->serialNumber, config->channel1Baudrate, config->channel2Baudrate);
	if (acquisitionSourceAddr == 0)
		return errno;

	uint64_t timestampCcAddr = writeTimestampCc (log->mdf);
	if (timestampCcAddr == 0)
		return errno;

	uint64_t dataFrameCgAddr = writeDataFrameCg (log->mdf, 0, acquisitionSourceAddr, timestampCcAddr);
	if (dataFrameCgAddr == 0)
		return errno;

	uint64_t fileHistoryAddr = writeFileHistory (log->mdf, config->timeStart, config->programId, config->softwareVersion);
	if (fileHistoryAddr == 0)
		return errno;

	uint64_t commentAddr = writeComment (log->mdf, config->softwareVersion, config->hardwareVersion, config->serialNumber,
		config->programId, config->storageSize, config->storageRemaining, config->sessionNumber, config->splitNumber);
	if (commentAddr == 0)
		return errno;

	mdfBlock_t* dg = writeDg (log->mdf, dataFrameCgAddr);
	if (dg == NULL)
		return errno;

	// Rewrite Link Lists -----------------------------------------------------------------------------------------------------

	uint64_t dtAddr = mdfDtBlockWrite (log->mdf);
	if (dtAddr == 0)
		return errno;

	mdfDgBlockLinkList (dg)->dataBlockAddr = dtAddr;

	if (mdfRewriteBlockLinkList (log->mdf, dg) != 0)
		return errno;

	mdfHdBlockLinkList (hd)->firstFhAddr = fileHistoryAddr;
	mdfHdBlockLinkList (hd)->commentAddr = commentAddr;
	mdfHdBlockLinkList (hd)->firstDgAddr = dg->addr;

	if (mdfRewriteBlockLinkList (log->mdf, hd) != 0)
		return errno;

	mdfBlockDealloc (dg);
	mdfBlockDealloc (hd);

	free (dg);
	free (hd);

	return 0;
}

int mdfCanBusLogWriteDataFrame (mdfCanBusLog_t* log, canFrame_t* frame, uint8_t busChannel, struct timeval* timestamp)
{
	// TODO(Barach): RTR?

	uint8_t record [20] = {};

	// Record ID
	record [0] = 0x01;

	// Timestamp
	uint64_t timestampInt = (timestamp->tv_sec - log->config->timeStart) * TIMESTAMP_SCALE_FACTOR + timestamp->tv_usec * TIMESTAMP_SCALE_FACTOR / 1e6;
	for (size_t index = 0; index < BIT_LENGTH_TO_BYTE_LENGTH (TIMESTAMP_BIT_LENGTH); ++index)
		record [index + TIMESTAMP_BYTE_OFFSET + 1] |= timestampInt >> (index * 8);

	// CAN ID
	for (size_t index = 0; index < BIT_LENGTH_TO_BYTE_LENGTH (ID_BIT_LENGTH); ++index)
		record [index + ID_BYTE_OFFSET + 1] |= frame->id >> (index * 8);

	// IDE
	record [IDE_BYTE_OFFSET + 1] |= (frame->ide & BIT_LENGTH_TO_BIT_MASK (IDE_BIT_LENGTH)) << IDE_BIT_OFFSET;

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
	if (fwrite (record, 1, sizeof (record), log->mdf) != sizeof (record))
		return errno;

	return 0;
}

int mdfCanBusLogClose (mdfCanBusLog_t *log)
{
	if (fclose (log->mdf) != 0)
		return errno;

	return 0;
}
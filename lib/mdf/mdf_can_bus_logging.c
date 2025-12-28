// Header
#include "mdf_can_bus_logging.h"

// Includes
#include "debug.h"
#include "mdf_writer.h"

// POSIX
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

// C Standard Library
#include <errno.h>
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>

// TODO(Barach): Move
long long int diff_timespec (const struct timespec* a, const struct timespec* b)
{
	return (a->tv_sec - b->tv_sec) * 1e9 + a->tv_nsec - b->tv_nsec;
}

// Macros ---------------------------------------------------------------------------------------------------------------------

#define BIT_LENGTH_TO_BYTE_LENGTH(bitLength) (((bitLength) + 7) / 8)
#define BIT_LENGTH_TO_BIT_MASK(bitLength) ((1 << (bitLength)) - 1)

#define TIMESTAMP_SCALE_FACTOR					1e9

// Conversions from can_device error code to MDF bus logging error type
#define ERROR_TYPE_BIT_ERROR					1
#define ERROR_TYPE_BIT_STUFF_ERROR				3
#define ERROR_TYPE_FORM_ERROR					2
#define ERROR_TYPE_ACK_ERROR					5
#define ERROR_TYPE_CRC_ERROR					4
#define ERROR_TYPE_BUS_OFF						6
#define ERROR_TYPE_UNSPEC_ERROR					0

// Constants ------------------------------------------------------------------------------------------------------------------

/// @brief The maximum size of an MDF log split (single file), in bytes.
// TODO(Barach): Actual max
#define MDF_SPLIT_SIZE_MAX						8600

// CAN Data Frame Record ------------------------------------------------------------------------------------------------------

#define DATA_FRAME_RECORD_ID					0x01

#define DATA_FRAME_BIT_LENGTH					104
#define DATA_FRAME_BYTE_OFFSET					6

#define DATA_FRAME_TIMESTAMP_BYTE_OFFSET		0
#define DATA_FRAME_TIMESTAMP_BIT_LENGTH			48

#define DATA_FRAME_ID_BYTE_OFFSET				6
#define DATA_FRAME_ID_BIT_LENGTH				29

#define DATA_FRAME_IDE_BYTE_OFFSET				9
#define DATA_FRAME_IDE_BIT_OFFSET				5
#define DATA_FRAME_IDE_BIT_LENGTH				1

#define DATA_FRAME_BUS_CHANNEL_BYTE_OFFSET		9
#define DATA_FRAME_BUS_CHANNEL_BIT_OFFSET		6
#define DATA_FRAME_BUS_CHANNEL_BIT_LENGTH		2

#define DATA_FRAME_DLC_BYTE_OFFSET				10
#define DATA_FRAME_DLC_BIT_OFFSET				0
#define DATA_FRAME_DLC_BIT_LENGTH				4

#define DATA_FRAME_DIR_BYTE_OFFSET				10
#define DATA_FRAME_DIR_BIT_OFFSET				4
#define DATA_FRAME_DIR_BIT_LENGTH				1

#define DATA_FRAME_DATA_BYTES_BYTE_OFFSET		11
#define DATA_FRAME_DATA_BYTES_BIT_LENGTH		64

// CAN Remote Frame Record ----------------------------------------------------------------------------------------------------

#define REMOTE_FRAME_RECORD_ID					0x02

#define REMOTE_FRAME_BIT_LENGTH					104
#define REMOTE_FRAME_BYTE_OFFSET				6

#define REMOTE_FRAME_TIMESTAMP_BYTE_OFFSET		0
#define REMOTE_FRAME_TIMESTAMP_BIT_LENGTH		48

#define REMOTE_FRAME_ID_BYTE_OFFSET				6
#define REMOTE_FRAME_ID_BIT_LENGTH				29

#define REMOTE_FRAME_IDE_BYTE_OFFSET			9
#define REMOTE_FRAME_IDE_BIT_OFFSET				5
#define REMOTE_FRAME_IDE_BIT_LENGTH				1

#define REMOTE_FRAME_BUS_CHANNEL_BYTE_OFFSET	9
#define REMOTE_FRAME_BUS_CHANNEL_BIT_OFFSET		6
#define REMOTE_FRAME_BUS_CHANNEL_BIT_LENGTH		2

#define REMOTE_FRAME_DLC_BYTE_OFFSET			10
#define REMOTE_FRAME_DLC_BIT_OFFSET				0
#define REMOTE_FRAME_DLC_BIT_LENGTH				4

#define REMOTE_FRAME_DIR_BYTE_OFFSET			10
#define REMOTE_FRAME_DIR_BIT_OFFSET				4
#define REMOTE_FRAME_DIR_BIT_LENGTH				1

#define REMOTE_FRAME_DATA_BYTES_BYTE_OFFSET		11
#define REMOTE_FRAME_DATA_BYTES_BIT_LENGTH		64

// CAN Error Frame Record -----------------------------------------------------------------------------------------------------

#define ERROR_FRAME_RECORD_ID					0x03

#define ERROR_FRAME_BIT_LENGTH					110
#define ERROR_FRAME_BYTE_OFFSET					6

#define ERROR_FRAME_TIMESTAMP_BYTE_OFFSET		0
#define ERROR_FRAME_TIMESTAMP_BIT_LENGTH		48

#define ERROR_FRAME_ID_BYTE_OFFSET				6
#define ERROR_FRAME_ID_BIT_LENGTH				29

#define ERROR_FRAME_IDE_BYTE_OFFSET				9
#define ERROR_FRAME_IDE_BIT_OFFSET				5
#define ERROR_FRAME_IDE_BIT_LENGTH				1

#define ERROR_FRAME_BUS_CHANNEL_BYTE_OFFSET		9
#define ERROR_FRAME_BUS_CHANNEL_BIT_OFFSET		6
#define ERROR_FRAME_BUS_CHANNEL_BIT_LENGTH		2

#define ERROR_FRAME_DLC_BYTE_OFFSET				10
#define ERROR_FRAME_DLC_BIT_OFFSET				0
#define ERROR_FRAME_DLC_BIT_LENGTH				4

#define ERROR_FRAME_DIR_BYTE_OFFSET				10
#define ERROR_FRAME_DIR_BIT_OFFSET				4
#define ERROR_FRAME_DIR_BIT_LENGTH				1

#define ERROR_FRAME_DATA_BYTES_BYTE_OFFSET		11
#define ERROR_FRAME_DATA_BYTES_BIT_LENGTH		64

#define ERROR_FRAME_ERROR_TYPE_BYTE_OFFSET		19
#define ERROR_FRAME_ERROR_TYPE_BIT_OFFSET		0
#define ERROR_FRAME_ERROR_TYPE_BIT_LENGTH		6

// Functions ------------------------------------------------------------------------------------------------------------------

static mdfBlock_t* writeHeader (FILE* mdf, const char* programId, time_t dateStart)
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
			.unixTimeNs = dateStart * 1e9 // File timestamp
		},
		&(mdfHdLinkList_t)
		{
			// Link list is populated later, as we don't know the addresses of any of these yet.
			0
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
			.byteOffset		= DATA_FRAME_DATA_BYTES_BYTE_OFFSET,
			.bitLength		= DATA_FRAME_DATA_BYTES_BIT_LENGTH,
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
			.bitOffset		= DATA_FRAME_DIR_BIT_OFFSET,
			.byteOffset		= DATA_FRAME_DIR_BYTE_OFFSET,
			.bitLength		= DATA_FRAME_DIR_BIT_LENGTH,
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
			.bitOffset		= DATA_FRAME_DLC_BIT_OFFSET,
			.byteOffset		= DATA_FRAME_DLC_BYTE_OFFSET,
			.bitLength		= DATA_FRAME_DLC_BIT_LENGTH,
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
			.bitOffset		= DATA_FRAME_DLC_BIT_OFFSET,
			.byteOffset		= DATA_FRAME_DLC_BYTE_OFFSET,
			.bitLength		= DATA_FRAME_DLC_BIT_LENGTH,
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
			.bitOffset		= DATA_FRAME_BUS_CHANNEL_BIT_OFFSET,
			.byteOffset		= DATA_FRAME_BUS_CHANNEL_BYTE_OFFSET,
			.bitLength		= DATA_FRAME_BUS_CHANNEL_BIT_LENGTH,
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
			.bitOffset		= DATA_FRAME_IDE_BIT_OFFSET,
			.byteOffset		= DATA_FRAME_IDE_BYTE_OFFSET,
			.bitLength		= DATA_FRAME_IDE_BIT_LENGTH,
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
			.byteOffset		= DATA_FRAME_ID_BYTE_OFFSET,
			.bitLength		= DATA_FRAME_ID_BIT_LENGTH,
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
			.byteOffset		= DATA_FRAME_BYTE_OFFSET,
			.bitLength		= DATA_FRAME_BIT_LENGTH,
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
			.byteOffset		= DATA_FRAME_TIMESTAMP_BYTE_OFFSET,
			.bitLength		= DATA_FRAME_TIMESTAMP_BIT_LENGTH,
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
			.recordId		= DATA_FRAME_RECORD_ID,
			.flags			= MDF_CG_FLAGS_BUS_EVENT | MDF_CG_FLAGS_PLAIN_BUS_EVENT,
			.pathSeparator	= '.',
			.byteLength		= BIT_LENGTH_TO_BYTE_LENGTH (DATA_FRAME_BIT_LENGTH + DATA_FRAME_TIMESTAMP_BIT_LENGTH)
		},
		&(mdfCgLinkList_t)
		{
			.nextCgAddr				= nextCgAddr,
			.firstCnAddr			= timestampCnAddr,
			.acquisitionNameAddr	= dataFrameNameAddr,
			.acquisitionSourceAddr	= acquisitionSourceAddr
		});
}

static uint64_t writeRemoteFrameCg (FILE* mdf, uint64_t nextCgAddr, uint64_t acquisitionSourceAddr, uint64_t timestampCcAddr)
{
	// Component Channels -----------------------------------------------------------------------------------------------------

	uint64_t dataBytesNameAddr = mdfTxBlockWrite (mdf, "CAN_RemoteFrame.DataBytes");
	if (dataBytesNameAddr == 0)
		return 0;

	uint64_t dataBytesCnAddr = mdfCnBlockWrite (mdf,
		&(mdfCnDataSection_t)
		{
			.channelType	= MDF_CHANNEL_TYPE_VALUE,
			.syncType		= MDF_SYNC_TYPE_NONE,
			.dataType		= MDF_DATA_TYPE_BYTE_ARRAY,
			.bitOffset		= 0,
			.byteOffset		= REMOTE_FRAME_DATA_BYTES_BYTE_OFFSET,
			.bitLength		= REMOTE_FRAME_DATA_BYTES_BIT_LENGTH,
			.flags			= MDF_CN_FLAGS_BUS_EVENT
		},
		&(mdfCnLinkList_t)
		{
			.nameAddr	= dataBytesNameAddr,
			.nextCnAddr	= 0
		});
	if (dataBytesCnAddr == 0)
		return 0;

	uint64_t dirNameAddr = mdfTxBlockWrite (mdf, "CAN_RemoteFrame.Dir");
	if (dirNameAddr == 0)
		return 0;

	uint64_t dirCnAddr = mdfCnBlockWrite (mdf,
		&(mdfCnDataSection_t)
		{
			.channelType	= MDF_CHANNEL_TYPE_VALUE,
			.syncType		= MDF_SYNC_TYPE_NONE,
			.dataType		= MDF_DATA_TYPE_UNSIGNED_INTEL,
			.bitOffset		= REMOTE_FRAME_DIR_BIT_OFFSET,
			.byteOffset		= REMOTE_FRAME_DIR_BYTE_OFFSET,
			.bitLength		= REMOTE_FRAME_DIR_BIT_LENGTH,
			.flags			= MDF_CN_FLAGS_BUS_EVENT
		},
		&(mdfCnLinkList_t)
		{
			.nameAddr	= dirNameAddr,
			.nextCnAddr	= dataBytesCnAddr
		});
	if (dirCnAddr == 0)
		return 0;

	uint64_t dataLengthNameAddr = mdfTxBlockWrite (mdf, "CAN_RemoteFrame.DataLength");
	if (dataLengthNameAddr == 0)
		return 0;

	uint64_t dataLengthCnAddr = mdfCnBlockWrite (mdf,
		&(mdfCnDataSection_t)
		{
			.channelType	= MDF_CHANNEL_TYPE_VALUE,
			.syncType		= MDF_SYNC_TYPE_NONE,
			.dataType		= MDF_DATA_TYPE_UNSIGNED_INTEL,
			.bitOffset		= REMOTE_FRAME_DLC_BIT_OFFSET,
			.byteOffset		= REMOTE_FRAME_DLC_BYTE_OFFSET,
			.bitLength		= REMOTE_FRAME_DLC_BIT_LENGTH,
			.flags			= MDF_CN_FLAGS_BUS_EVENT
		},
		&(mdfCnLinkList_t)
		{
			.nameAddr	= dataLengthNameAddr,
			.nextCnAddr	= dirCnAddr
		});
	if (dataLengthCnAddr == 0)
		return 0;

	uint64_t dlcNameAddr = mdfTxBlockWrite (mdf, "CAN_RemoteFrame.DLC");
	if (dlcNameAddr == 0)
		return 0;

	uint64_t dlcCnAddr = mdfCnBlockWrite (mdf,
		&(mdfCnDataSection_t)
		{
			.channelType	= MDF_CHANNEL_TYPE_VALUE,
			.syncType		= MDF_SYNC_TYPE_NONE,
			.dataType		= MDF_DATA_TYPE_UNSIGNED_INTEL,
			.bitOffset		= REMOTE_FRAME_DLC_BIT_OFFSET,
			.byteOffset		= REMOTE_FRAME_DLC_BYTE_OFFSET,
			.bitLength		= REMOTE_FRAME_DLC_BIT_LENGTH,
			.flags			= MDF_CN_FLAGS_BUS_EVENT
		},
		&(mdfCnLinkList_t)
		{
			.nameAddr	= dlcNameAddr,
			.nextCnAddr	= dataLengthCnAddr
		});
	if (dlcCnAddr == 0)
		return 0;

	uint64_t busChannelNameAddr = mdfTxBlockWrite (mdf, "CAN_RemoteFrame.BusChannel");
	if (busChannelNameAddr == 0)
		return 0;

	uint64_t busChannelCnAddr = mdfCnBlockWrite (mdf,
		&(mdfCnDataSection_t)
		{
			.channelType	= MDF_CHANNEL_TYPE_VALUE,
			.syncType		= MDF_SYNC_TYPE_NONE,
			.dataType		= MDF_DATA_TYPE_UNSIGNED_INTEL,
			.bitOffset		= REMOTE_FRAME_BUS_CHANNEL_BIT_OFFSET,
			.byteOffset		= REMOTE_FRAME_BUS_CHANNEL_BYTE_OFFSET,
			.bitLength		= REMOTE_FRAME_BUS_CHANNEL_BIT_LENGTH,
			.flags			= MDF_CN_FLAGS_BUS_EVENT
		},
		&(mdfCnLinkList_t)
		{
			.nameAddr	= busChannelNameAddr,
			.nextCnAddr	= dlcCnAddr
		});
	if (busChannelCnAddr == 0)
		return 0;

	uint64_t ideNameAddr = mdfTxBlockWrite (mdf, "CAN_RemoteFrame.IDE");
	if (ideNameAddr == 0)
		return 0;

	uint64_t ideCnAddr = mdfCnBlockWrite (mdf,
		&(mdfCnDataSection_t)
		{
			.channelType	= MDF_CHANNEL_TYPE_VALUE,
			.syncType		= MDF_SYNC_TYPE_NONE,
			.dataType		= MDF_DATA_TYPE_UNSIGNED_INTEL,
			.bitOffset		= REMOTE_FRAME_IDE_BIT_OFFSET,
			.byteOffset		= REMOTE_FRAME_IDE_BYTE_OFFSET,
			.bitLength		= REMOTE_FRAME_IDE_BIT_LENGTH,
			.flags			= MDF_CN_FLAGS_BUS_EVENT
		},
		&(mdfCnLinkList_t)
		{
			.nameAddr	= ideNameAddr,
			.nextCnAddr	= busChannelCnAddr
		});
	if (ideCnAddr == 0)
		return 0;

	uint64_t idNameAddr = mdfTxBlockWrite (mdf, "CAN_RemoteFrame.ID");
	if (idNameAddr == 0)
		return 0;

	uint64_t idCnAddr = mdfCnBlockWrite (mdf,
		&(mdfCnDataSection_t)
		{
			.channelType	= MDF_CHANNEL_TYPE_VALUE,
			.syncType		= MDF_SYNC_TYPE_NONE,
			.dataType		= MDF_DATA_TYPE_UNSIGNED_INTEL,
			.bitOffset		= 0,
			.byteOffset		= REMOTE_FRAME_ID_BYTE_OFFSET,
			.bitLength		= REMOTE_FRAME_ID_BIT_LENGTH,
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

	uint64_t dataFrameNameAddr = mdfTxBlockWrite (mdf, "CAN_RemoteFrame");
	if (dataFrameNameAddr == 0)
		return 0;

	uint64_t dataFrameCnAddr = mdfCnBlockWrite (mdf,
		&(mdfCnDataSection_t)
		{
			.channelType	= MDF_CHANNEL_TYPE_VALUE,
			.syncType		= MDF_SYNC_TYPE_NONE,
			.dataType		= MDF_DATA_TYPE_BYTE_ARRAY,
			.bitOffset		= 0,
			.byteOffset		= REMOTE_FRAME_BYTE_OFFSET,
			.bitLength		= REMOTE_FRAME_BIT_LENGTH,
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
			.byteOffset		= REMOTE_FRAME_TIMESTAMP_BYTE_OFFSET,
			.bitLength		= REMOTE_FRAME_TIMESTAMP_BIT_LENGTH,
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
			.recordId		= REMOTE_FRAME_RECORD_ID,
			.flags			= MDF_CG_FLAGS_BUS_EVENT | MDF_CG_FLAGS_PLAIN_BUS_EVENT,
			.pathSeparator	= '.',
			.byteLength		= BIT_LENGTH_TO_BYTE_LENGTH (REMOTE_FRAME_BIT_LENGTH + REMOTE_FRAME_TIMESTAMP_BIT_LENGTH)
		},
		&(mdfCgLinkList_t)
		{
			.nextCgAddr				= nextCgAddr,
			.firstCnAddr			= timestampCnAddr,
			.acquisitionNameAddr	= dataFrameNameAddr,
			.acquisitionSourceAddr	= acquisitionSourceAddr
		});
}

static uint64_t writeErrorFrameCg (FILE* mdf, uint64_t nextCgAddr, uint64_t acquisitionSourceAddr, uint64_t timestampCcAddr)
{
	// Component Channels -----------------------------------------------------------------------------------------------------

	uint64_t errorTypeNameAddr = mdfTxBlockWrite (mdf, "CAN_ErrorFrame.ErrorType");
	if (errorTypeNameAddr == 0)
		return 0;

	uint64_t errorTypeCnAddr = mdfCnBlockWrite (mdf,
		&(mdfCnDataSection_t)
		{
			.channelType	= MDF_CHANNEL_TYPE_VALUE,
			.syncType		= MDF_SYNC_TYPE_NONE,
			.dataType		= MDF_DATA_TYPE_UNSIGNED_INTEL,
			.bitOffset		= ERROR_FRAME_ERROR_TYPE_BIT_OFFSET,
			.byteOffset		= ERROR_FRAME_ERROR_TYPE_BYTE_OFFSET,
			.bitLength		= ERROR_FRAME_ERROR_TYPE_BIT_LENGTH,
			.flags			= MDF_CN_FLAGS_BUS_EVENT
		},
		&(mdfCnLinkList_t)
		{
			.nameAddr	= errorTypeNameAddr,
			.nextCnAddr	= 0
		});
	if (errorTypeCnAddr == 0)
		return 0;

	// Write the name block
	uint64_t dataBytesNameAddr = mdfTxBlockWrite (mdf, "CAN_ErrorFrame.DataBytes");
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
			.byteOffset		= ERROR_FRAME_DATA_BYTES_BYTE_OFFSET,
			.bitLength		= ERROR_FRAME_DATA_BYTES_BIT_LENGTH,
			.flags			= MDF_CN_FLAGS_BUS_EVENT
		},
		&(mdfCnLinkList_t)
		{
			.nameAddr	= dataBytesNameAddr,
			.nextCnAddr	= errorTypeCnAddr
		});
	if (dataBytesCnAddr == 0)
		return 0;

	uint64_t dirNameAddr = mdfTxBlockWrite (mdf, "CAN_ErrorFrame.Dir");
	if (dirNameAddr == 0)
		return 0;

	uint64_t dirCnAddr = mdfCnBlockWrite (mdf,
		&(mdfCnDataSection_t)
		{
			.channelType	= MDF_CHANNEL_TYPE_VALUE,
			.syncType		= MDF_SYNC_TYPE_NONE,
			.dataType		= MDF_DATA_TYPE_UNSIGNED_INTEL,
			.bitOffset		= ERROR_FRAME_DIR_BIT_OFFSET,
			.byteOffset		= ERROR_FRAME_DIR_BYTE_OFFSET,
			.bitLength		= ERROR_FRAME_DIR_BIT_LENGTH,
			.flags			= MDF_CN_FLAGS_BUS_EVENT
		},
		&(mdfCnLinkList_t)
		{
			.nameAddr	= dirNameAddr,
			.nextCnAddr	= dataBytesCnAddr
		});
	if (dirCnAddr == 0)
		return 0;

	uint64_t dataLengthNameAddr = mdfTxBlockWrite (mdf, "CAN_ErrorFrame.DataLength");
	if (dataLengthNameAddr == 0)
		return 0;

	uint64_t dataLengthCnAddr = mdfCnBlockWrite (mdf,
		&(mdfCnDataSection_t)
		{
			.channelType	= MDF_CHANNEL_TYPE_VALUE,
			.syncType		= MDF_SYNC_TYPE_NONE,
			.dataType		= MDF_DATA_TYPE_UNSIGNED_INTEL,
			.bitOffset		= ERROR_FRAME_DLC_BIT_OFFSET,
			.byteOffset		= ERROR_FRAME_DLC_BYTE_OFFSET,
			.bitLength		= ERROR_FRAME_DLC_BIT_LENGTH,
			.flags			= MDF_CN_FLAGS_BUS_EVENT
		},
		&(mdfCnLinkList_t)
		{
			.nameAddr	= dataLengthNameAddr,
			.nextCnAddr	= dirCnAddr
		});
	if (dataLengthCnAddr == 0)
		return 0;

	uint64_t dlcNameAddr = mdfTxBlockWrite (mdf, "CAN_ErrorFrame.DLC");
	if (dlcNameAddr == 0)
		return 0;

	uint64_t dlcCnAddr = mdfCnBlockWrite (mdf,
		&(mdfCnDataSection_t)
		{
			.channelType	= MDF_CHANNEL_TYPE_VALUE,
			.syncType		= MDF_SYNC_TYPE_NONE,
			.dataType		= MDF_DATA_TYPE_UNSIGNED_INTEL,
			.bitOffset		= ERROR_FRAME_DLC_BIT_OFFSET,
			.byteOffset		= ERROR_FRAME_DLC_BYTE_OFFSET,
			.bitLength		= ERROR_FRAME_DLC_BIT_LENGTH,
			.flags			= MDF_CN_FLAGS_BUS_EVENT
		},
		&(mdfCnLinkList_t)
		{
			.nameAddr	= dlcNameAddr,
			.nextCnAddr	= dataLengthCnAddr
		});
	if (dlcCnAddr == 0)
		return 0;

	uint64_t busChannelNameAddr = mdfTxBlockWrite (mdf, "CAN_ErrorFrame.BusChannel");
	if (busChannelNameAddr == 0)
		return 0;

	uint64_t busChannelCnAddr = mdfCnBlockWrite (mdf,
		&(mdfCnDataSection_t)
		{
			.channelType	= MDF_CHANNEL_TYPE_VALUE,
			.syncType		= MDF_SYNC_TYPE_NONE,
			.dataType		= MDF_DATA_TYPE_UNSIGNED_INTEL,
			.bitOffset		= ERROR_FRAME_BUS_CHANNEL_BIT_OFFSET,
			.byteOffset		= ERROR_FRAME_BUS_CHANNEL_BYTE_OFFSET,
			.bitLength		= ERROR_FRAME_BUS_CHANNEL_BIT_LENGTH,
			.flags			= MDF_CN_FLAGS_BUS_EVENT
		},
		&(mdfCnLinkList_t)
		{
			.nameAddr	= busChannelNameAddr,
			.nextCnAddr	= dlcCnAddr
		});
	if (busChannelCnAddr == 0)
		return 0;

	uint64_t ideNameAddr = mdfTxBlockWrite (mdf, "CAN_ErrorFrame.IDE");
	if (ideNameAddr == 0)
		return 0;

	uint64_t ideCnAddr = mdfCnBlockWrite (mdf,
		&(mdfCnDataSection_t)
		{
			.channelType	= MDF_CHANNEL_TYPE_VALUE,
			.syncType		= MDF_SYNC_TYPE_NONE,
			.dataType		= MDF_DATA_TYPE_UNSIGNED_INTEL,
			.bitOffset		= ERROR_FRAME_IDE_BIT_OFFSET,
			.byteOffset		= ERROR_FRAME_IDE_BYTE_OFFSET,
			.bitLength		= ERROR_FRAME_IDE_BIT_LENGTH,
			.flags			= MDF_CN_FLAGS_BUS_EVENT
		},
		&(mdfCnLinkList_t)
		{
			.nameAddr	= ideNameAddr,
			.nextCnAddr	= busChannelCnAddr
		});
	if (ideCnAddr == 0)
		return 0;

	uint64_t idNameAddr = mdfTxBlockWrite (mdf, "CAN_ErrorFrame.ID");
	if (idNameAddr == 0)
		return 0;

	uint64_t idCnAddr = mdfCnBlockWrite (mdf,
		&(mdfCnDataSection_t)
		{
			.channelType	= MDF_CHANNEL_TYPE_VALUE,
			.syncType		= MDF_SYNC_TYPE_NONE,
			.dataType		= MDF_DATA_TYPE_UNSIGNED_INTEL,
			.bitOffset		= 0,
			.byteOffset		= ERROR_FRAME_ID_BYTE_OFFSET,
			.bitLength		= ERROR_FRAME_ID_BIT_LENGTH,
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

	uint64_t dataFrameNameAddr = mdfTxBlockWrite (mdf, "CAN_ErrorFrame");
	if (dataFrameNameAddr == 0)
		return 0;

	uint64_t dataFrameCnAddr = mdfCnBlockWrite (mdf,
		&(mdfCnDataSection_t)
		{
			.channelType	= MDF_CHANNEL_TYPE_VALUE,
			.syncType		= MDF_SYNC_TYPE_NONE,
			.dataType		= MDF_DATA_TYPE_BYTE_ARRAY,
			.bitOffset		= 0,
			.byteOffset		= ERROR_FRAME_BYTE_OFFSET,
			.bitLength		= ERROR_FRAME_BIT_LENGTH,
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
			.byteOffset		= ERROR_FRAME_TIMESTAMP_BYTE_OFFSET,
			.bitLength		= ERROR_FRAME_TIMESTAMP_BIT_LENGTH,
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
			.recordId		= ERROR_FRAME_RECORD_ID,
			.flags			= MDF_CG_FLAGS_BUS_EVENT | MDF_CG_FLAGS_PLAIN_BUS_EVENT,
			.pathSeparator	= '.',
			.byteLength		= BIT_LENGTH_TO_BYTE_LENGTH (ERROR_FRAME_BIT_LENGTH + ERROR_FRAME_TIMESTAMP_BIT_LENGTH)
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

static uint64_t writeFileHistory (FILE* mdf, time_t dateStart, const char* softwareName, const char* softwareVersion, const char* softwareVendor)
{
	uint64_t commentAddr = mdfMdBlockWrite (mdf,
		"<FHcomment>\n"
		"	<TX>\n"
		"		Creation and logging of data.\n"
		"	</TX>\n"
		"	<tool_id>%s</tool_id>\n"
		"	<tool_vendor>%s</tool_vendor>\n"
		"	<tool_version>%s</tool_version>\n"
		"</FHcomment>", softwareName, softwareVendor, softwareVersion);
	if (commentAddr == 0)
		return 0;

	return mdfFhBlockWrite (mdf,
		&(mdfFhDataSection_t)
		{
			.unixTimeNs = dateStart * 1e9
		},
		&(mdfFhLinkList_t)
		{
			.commentAddr = commentAddr
		});
}

static uint64_t writeComment (FILE* mdf, const char* configurationName, const char* softwareVersion,
	const char* hardwareVersion, const char* serialNumber, const char* hardwareName, size_t storageSize,
	size_t storageRemaining, uint32_t sessionNumber, uint32_t splitNumber)
{
	return mdfMdBlockWrite (mdf,
		"<HDcomment>\n"
		"    <TX/>\n"
		"    <common_properties>\n"
		"        <tree name=\"Device Information\">\n"
		"            <e name=\"configuration name\" ro=\"true\">%s</e>\n"
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
		"</HDcomment>", configurationName, softwareVersion, hardwareVersion, serialNumber, hardwareName,
			(unsigned long) storageSize, (unsigned long) storageRemaining, sessionNumber, splitNumber);
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

static int createDestinationDirectory (const char* parentDirectory, uint32_t sessionNumber)
{
	// Create the directory path based on the session number
	char* path;
	if (asprintf (&path, "%s/session_%"PRIu32"", parentDirectory, sessionNumber) < 0)
		return errno;

	// Attempt to create the directory, if it fails (not due to already existing), return failure.
	debugPrintf ("Creating destination directory '%s'.\n", path);
	if (mkdir (path, S_IRWXU | S_IRGRP | S_IROTH) != 0 && errno != EEXIST)
		return errno;

	// Free the path string
	free (path);
	return 0;
}

static FILE* createDestinationFile (const char* parentDirectory, uint32_t sessionNumber, uint32_t splitNumber, char** splitName)
{
	// Create the file path based on the parent directory, session number, and split number.
	// - Note: The split name is deallocate when the file is closed.
	if (asprintf (splitName, "%s/session_%"PRIu32"/split_%"PRIu32".mf4", parentDirectory, sessionNumber, splitNumber) < 0)
		return NULL;

	// Attempt to create the file
	debugPrintf ("Creating destination file '%s'.\n", *splitName);
	return fopen (*splitName, "w");
}

static int createSplit (mdfCanBusLog_t* log, uint32_t splitNumber)
{
	log->splitNumber = splitNumber;

	// Create the destination file within said directory.
	log->mdf = createDestinationFile (log->config->directory, log->config->sessionNumber, log->splitNumber, &log->splitName);
	if (log->mdf == NULL)
		return errno;

	mdfBlock_t* hd = writeHeader (log->mdf, "ZREMDF", log->dateStart);
	if (hd == NULL)
		return errno;

	uint64_t acquisitionSourceAddr = writeAcquisitionSource (log->mdf, log->config->softwareVersion,
		log->config->hardwareVersion, log->config->serialNumber, log->config->channel1Baudrate,
		log->config->channel2Baudrate);
	if (acquisitionSourceAddr == 0)
		return errno;

	uint64_t timestampCcAddr = writeTimestampCc (log->mdf);
	if (timestampCcAddr == 0)
		return errno;

	uint64_t errorFrameCg = writeErrorFrameCg (log->mdf, 0, acquisitionSourceAddr, timestampCcAddr);
	if (errorFrameCg == 0)
		return errno;

	uint64_t remoteFrameCgAddr = writeRemoteFrameCg (log->mdf, errorFrameCg, acquisitionSourceAddr, timestampCcAddr);
	if (remoteFrameCgAddr == 0)
		return errno;

	uint64_t dataFrameCgAddr = writeDataFrameCg (log->mdf, remoteFrameCgAddr, acquisitionSourceAddr, timestampCcAddr);
	if (dataFrameCgAddr == 0)
		return errno;

	uint64_t fileHistoryAddr = writeFileHistory (log->mdf, log->dateStart, log->config->softwareName,
		log->config->softwareVersion, log->config->softwareVendor);
	if (fileHistoryAddr == 0)
		return errno;

	uint64_t commentAddr = writeComment (log->mdf, log->config->configurationName, log->config->softwareVersion,
		log->config->hardwareVersion, log->config->serialNumber, log->config->hardwareName, log->config->storageSize,
		log->config->storageRemaining, log->config->sessionNumber, log->splitNumber);
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

	// Get header size
	long splitSize = ftell (log->mdf);
	if (splitSize < 0)
		return errno;

	log->splitSize = (size_t) splitSize;

	return 0;
}

static void closeSplit (mdfCanBusLog_t* log)
{
	free (log->splitName);
	fclose (log->mdf);
}

static int writeRecord (mdfCanBusLog_t* log, uint8_t* record, size_t recordSize)
{
	if (log->splitSize + recordSize > MDF_SPLIT_SIZE_MAX)
	{
		debugPrintf ("MDF split size exceeds maximum. Splitting log... ");
		closeSplit (log);
		if (createSplit (log, log->splitNumber + 1) != 0)
			return errno;
		debugPrintf ("Success.\n");
	}

	if (fwrite (record, 1, recordSize, log->mdf) != recordSize)
		return errno;

	log->splitSize += recordSize;
	debugPrintf ("Log size: %lu bytes.\n", (unsigned long) log->splitSize);

	return 0;
}

uint32_t mdfCanBusLogGetSessionNumber (const char* directory)
{
	debugPrintf ("Searching for MDF session number...\n");

	DIR* dirp = opendir (directory);
	if (dirp == NULL)
	{
		debugPrintf ("    Warning: MDF destination directory '%s' does not exist. Assuming session number 0.\n", directory);
		return 0;
	}

	uint32_t sessionNumber = 0;
	while (true)
	{
		struct dirent* ent = readdir (dirp);
		if (ent == NULL)
			break;

		if (strncmp (ent->d_name, "session_", strlen ("session_")) == 0)
		{
			char* entSessionNumberStr = ent->d_name + strlen ("session_");
			char* endPtr;
			uint32_t entSessionNumber = strtoul (entSessionNumberStr, &endPtr, 0);
			if (endPtr != entSessionNumberStr && entSessionNumber >= sessionNumber)
			{
				sessionNumber = entSessionNumber + 1;
				debugPrintf ("    Session number %"PRIu32" found, current max is %"PRIu32".\n", entSessionNumber, sessionNumber);
			}
		}
		else
			debugPrintf ("    Directory entry '%s' does not appear to be a log session, skipping...\n", ent->d_name);
	}

	debugPrintf ("MDF session number assumed to be %"PRIu32".\n", sessionNumber);

	closedir (dirp);
	return sessionNumber;
}

int mdfCanBusLogInit (mdfCanBusLog_t* log, const mdfCanBusLogConfig_t* config)
{
	log->config = config;

	// Get the date and time of the log file.
	log->dateStart = time (NULL);
	clock_gettime (CLOCK_MONOTONIC, &log->timeStart);

	// Create the destination directory.
	if (createDestinationDirectory (config->directory, config->sessionNumber) != 0)
		return errno;

	if (createSplit (log, 0) != 0)
		return errno;

	debugPrintf ("Initial MDF split size: %lu bytes.\n", (long unsigned) log->splitSize);
	return 0;
}

const char* mdfCanBusLogGetName (mdfCanBusLog_t* log)
{
	return log->splitName;
}

int mdfCanBusLogWriteDataFrame (mdfCanBusLog_t* log, canFrame_t* frame, uint8_t busChannel, bool direction, struct timespec* timestamp)
{
	uint8_t record [BIT_LENGTH_TO_BYTE_LENGTH (DATA_FRAME_BIT_LENGTH + DATA_FRAME_TIMESTAMP_BIT_LENGTH) + 1] = {0};

	// Record ID
	record [0] = DATA_FRAME_RECORD_ID;

	// Timestamp
	long long timestampInt = diff_timespec (timestamp, &log->timeStart);
	for (size_t index = 0; index < BIT_LENGTH_TO_BYTE_LENGTH (DATA_FRAME_TIMESTAMP_BIT_LENGTH); ++index)
		record [index + DATA_FRAME_TIMESTAMP_BYTE_OFFSET + 1] |= timestampInt >> (index * 8);

	// CAN ID
	for (size_t index = 0; index < BIT_LENGTH_TO_BYTE_LENGTH (DATA_FRAME_ID_BIT_LENGTH); ++index)
		record [index + DATA_FRAME_ID_BYTE_OFFSET + 1] |= frame->id >> (index * 8);

	// IDE
	record [DATA_FRAME_IDE_BYTE_OFFSET + 1] |= (frame->ide & BIT_LENGTH_TO_BIT_MASK (DATA_FRAME_IDE_BIT_LENGTH))
		<< DATA_FRAME_IDE_BIT_OFFSET;

	// Bus channel
	record [DATA_FRAME_BUS_CHANNEL_BYTE_OFFSET + 1] |=
		(busChannel & BIT_LENGTH_TO_BIT_MASK (DATA_FRAME_BUS_CHANNEL_BIT_LENGTH)) << DATA_FRAME_BUS_CHANNEL_BIT_OFFSET;

	// DLC
	record [DATA_FRAME_DLC_BYTE_OFFSET + 1] |=
		(frame->dlc & BIT_LENGTH_TO_BIT_MASK (DATA_FRAME_DLC_BIT_LENGTH)) << DATA_FRAME_DLC_BIT_OFFSET;

	// DIR
	record [DATA_FRAME_DIR_BYTE_OFFSET + 1] |= direction << DATA_FRAME_DIR_BIT_OFFSET;

	// Data Bytes
	for (size_t index = 0; index < frame->dlc; ++index)
		record [DATA_FRAME_DATA_BYTES_BYTE_OFFSET + index + 1] |= frame->data [index];

	// Write the record to the file.
	if (writeRecord (log, record, sizeof (record)) != 0)
		return errno;

	return 0;
}

int mdfCanBusLogWriteRemoteFrame (mdfCanBusLog_t* log, canFrame_t* frame, uint8_t busChannel, bool direction, struct timespec* timestamp)
{
	uint8_t record [BIT_LENGTH_TO_BYTE_LENGTH (REMOTE_FRAME_BIT_LENGTH + REMOTE_FRAME_TIMESTAMP_BIT_LENGTH) + 1] = {0};

	// Record ID
	record [0] = REMOTE_FRAME_RECORD_ID;

	// Timestamp
	long long timestampInt = diff_timespec (timestamp, &log->timeStart);
	for (size_t index = 0; index < BIT_LENGTH_TO_BYTE_LENGTH (REMOTE_FRAME_TIMESTAMP_BIT_LENGTH); ++index)
		record [index + REMOTE_FRAME_TIMESTAMP_BYTE_OFFSET + 1] |= timestampInt >> (index * 8);

	// CAN ID
	for (size_t index = 0; index < BIT_LENGTH_TO_BYTE_LENGTH (REMOTE_FRAME_ID_BIT_LENGTH); ++index)
		record [index + REMOTE_FRAME_ID_BYTE_OFFSET + 1] |= frame->id >> (index * 8);

	// IDE
	record [REMOTE_FRAME_IDE_BYTE_OFFSET + 1] |= (frame->ide & BIT_LENGTH_TO_BIT_MASK (REMOTE_FRAME_IDE_BIT_LENGTH))
		<< REMOTE_FRAME_IDE_BIT_OFFSET;

	// Bus channel
	record [REMOTE_FRAME_BUS_CHANNEL_BYTE_OFFSET + 1] |=
		(busChannel & BIT_LENGTH_TO_BIT_MASK (REMOTE_FRAME_BUS_CHANNEL_BIT_LENGTH)) << REMOTE_FRAME_BUS_CHANNEL_BIT_OFFSET;

	// DLC
	record [REMOTE_FRAME_DLC_BYTE_OFFSET + 1] |=
		(frame->dlc & BIT_LENGTH_TO_BIT_MASK (REMOTE_FRAME_DLC_BIT_LENGTH)) << REMOTE_FRAME_DLC_BIT_OFFSET;

	// DIR
	record [REMOTE_FRAME_DIR_BYTE_OFFSET + 1] |= direction << REMOTE_FRAME_DIR_BIT_OFFSET;

	// Data Bytes
	for (size_t index = 0; index < frame->dlc; ++index)
		record [REMOTE_FRAME_DATA_BYTES_BYTE_OFFSET + index + 1] |= frame->data [index];

	// Write the record to the file.
	if (writeRecord (log, record, sizeof (record)) != 0)
		return errno;

	return 0;
}

int mdfCanBusLogWriteErrorFrame (mdfCanBusLog_t* log, canFrame_t* frame, uint8_t busChannel, bool direction, int errorCode, struct timespec* timestamp)
{
	uint8_t record [BIT_LENGTH_TO_BYTE_LENGTH (ERROR_FRAME_BIT_LENGTH + ERROR_FRAME_TIMESTAMP_BIT_LENGTH) + 1] = {0};

	// Record ID
	record [0] = ERROR_FRAME_RECORD_ID;

	// Timestamp
	long long timestampInt = diff_timespec (timestamp, &log->timeStart);
	for (size_t index = 0; index < BIT_LENGTH_TO_BYTE_LENGTH (ERROR_FRAME_TIMESTAMP_BIT_LENGTH); ++index)
		record [index + ERROR_FRAME_TIMESTAMP_BYTE_OFFSET + 1] |= timestampInt >> (index * 8);

	// CAN ID
	for (size_t index = 0; index < BIT_LENGTH_TO_BYTE_LENGTH (ERROR_FRAME_ID_BIT_LENGTH); ++index)
		record [index + ERROR_FRAME_ID_BYTE_OFFSET + 1] |= frame->id >> (index * 8);

	// IDE
	record [ERROR_FRAME_IDE_BYTE_OFFSET + 1] |= (frame->ide & BIT_LENGTH_TO_BIT_MASK (ERROR_FRAME_IDE_BIT_LENGTH))
		<< ERROR_FRAME_IDE_BIT_OFFSET;

	// Bus channel
	record [ERROR_FRAME_BUS_CHANNEL_BYTE_OFFSET + 1] |=
		(busChannel & BIT_LENGTH_TO_BIT_MASK (ERROR_FRAME_BUS_CHANNEL_BIT_LENGTH)) << ERROR_FRAME_BUS_CHANNEL_BIT_OFFSET;

	// DLC
	record [ERROR_FRAME_DLC_BYTE_OFFSET + 1] |=
		(frame->dlc & BIT_LENGTH_TO_BIT_MASK (ERROR_FRAME_DLC_BIT_LENGTH)) << ERROR_FRAME_DLC_BIT_OFFSET;

	// DIR
	record [ERROR_FRAME_DIR_BYTE_OFFSET + 1] |= direction << ERROR_FRAME_DIR_BIT_OFFSET;

	// Data Bytes
	for (size_t index = 0; index < frame->dlc; ++index)
		record [ERROR_FRAME_DATA_BYTES_BYTE_OFFSET + index + 1] |= frame->data [index];

	uint8_t errorType;
	switch (errorCode)
	{
	case ERRNO_CAN_DEVICE_BIT_ERROR:
		errorType = ERROR_TYPE_BIT_ERROR;
		break;

	case ERRNO_CAN_DEVICE_BIT_STUFF_ERROR:
		errorType = ERROR_TYPE_BIT_STUFF_ERROR;
		break;

	case ERRNO_CAN_DEVICE_FORM_ERROR:
		errorType = ERROR_TYPE_FORM_ERROR;
		break;

	case ERRNO_CAN_DEVICE_ACK_ERROR:
		errorType = ERROR_TYPE_ACK_ERROR;
		break;

	case ERRNO_CAN_DEVICE_CRC_ERROR:
		errorType = ERROR_TYPE_CRC_ERROR;
		break;

	case ERRNO_CAN_DEVICE_BUS_OFF:
		errorType = ERROR_TYPE_BUS_OFF;
		break;

	case ERRNO_CAN_DEVICE_UNSPEC_ERROR:
	default:
		errorType = ERROR_TYPE_UNSPEC_ERROR;
		break;
	}
	record [ERROR_FRAME_ERROR_TYPE_BYTE_OFFSET + 1] |=
		(errorType & BIT_LENGTH_TO_BIT_MASK (ERROR_FRAME_ERROR_TYPE_BIT_LENGTH)) << ERROR_FRAME_ERROR_TYPE_BIT_OFFSET;

	// Write the record to the file.
	if (writeRecord (log, record, sizeof (record)) != 0)
		return errno;

	return 0;
}

int mdfCanBusLogClose (mdfCanBusLog_t *log)
{
	if (fclose (log->mdf) != 0)
		return errno;

	return 0;
}
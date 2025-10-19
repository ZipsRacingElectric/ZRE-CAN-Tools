// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "mdf/mdf_writer.h"
#include "debug.h"
#include "error_codes.h"
#include "can_device/can_device.h"

// C Standard Library
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>

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

void writeCan1RxRecord (FILE* mdf, canFrame_t* frame, uint64_t timestamp, uint8_t busChannel)
{
	uint8_t record [20] = {};

	// Record ID
	record [0] = 0x01;

	// Timestamp
	for (size_t index = 0; index < BIT_LENGTH_TO_BYTE_LENGTH (TIMESTAMP_BIT_LENGTH); ++index)
		record [index + TIMESTAMP_BYTE_OFFSET + 1] = timestamp >> (index * 8);

	// CAN ID
	for (size_t index = 0; index < BIT_LENGTH_TO_BYTE_LENGTH (ID_BIT_LENGTH); ++index)
		record [index + ID_BYTE_OFFSET + 1] = frame->id >> (index * 8);

	// IDE
	record [IDE_BYTE_OFFSET + 1] = (0 & BIT_LENGTH_TO_BIT_MASK (IDE_BIT_LENGTH)) << IDE_BIT_OFFSET;

	// Bus channel
	record [BUS_CHANNEL_BYTE_OFFSET + 1] =
		(busChannel & BIT_LENGTH_TO_BIT_MASK (BUS_CHANNEL_BIT_LENGTH)) << BUS_CHANNEL_BIT_OFFSET;

	// DLC
	record [DLC_BYTE_OFFSET + 1] =
		(frame->dlc & BIT_LENGTH_TO_BIT_MASK (DLC_BIT_LENGTH)) << DLC_BIT_OFFSET;

	// Data Bytes
	for (size_t index = 0; index < frame->dlc; ++index)
		record [DATA_BYTES_BYTE_OFFSET + index + 1] = frame->data [index];

	fwrite (record, 1, sizeof (record), mdf);
}

// Entrypoint -----------------------------------------------------------------------------------------------------------------

int main (int argc, char** argv)
{
	debugInit ();

	if (argc < 2)
	{
		fprintf (stderr, "Invalid arguments, usage: mdf-stub <MDF file name>");
		return -1;
	}

	char* filePath = argv [argc - 1];
	FILE* mdf = fopen (filePath, "w");
	if (mdf == NULL)
	{
		int code = errno;
		fprintf (stderr, "Failed to open MDF file '%s': %s\n", filePath, errorMessage (code));
		return code;
	}

	// Header -----------------------------------------------------------------------------------------------------------------

	time_t timeUtc = time (NULL);
	time_t timeLocal = timeUtc + localtime (&timeUtc)->tm_gmtoff;

	mdfFileIdBlock_t fileIdBlock =
	{
		.fileIdentification		= MDF_FILE_IDENTIFICATION_UNFINALIZED,
		.versionString			= MDF_VERSION_STRING_V4_11,
		.programIdentification	= "ZREDART",
		.data					= MDF_FILE_ID_BLOCK_DATA
	};
	mdfWriteFileIdBlock (mdf, &fileIdBlock);

	mdfBlock_t hd;
	mdfHdBlockInit (&hd,
		&(mdfHdDataSection_t)
		{
			.unixTimeNs = timeLocal * 1e9
		},
		&(mdfHdLinkList_t) {});
	mdfBlockWrite (mdf, &hd);

	// CAN_DataFrame Component Channels ---------------------------------------------------------------------------------------

	uint64_t addrDataBytes = mdfCnBlockWrite (mdf,
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
			.nameAddr	= mdfTxBlockWrite (mdf, "CAN_DataFrame.DataBytes"),
			.nextCnAddr	= 0
		});

	uint64_t addrBrs = mdfCnBlockWrite (mdf,
		&(mdfCnDataSection_t)
		{
			.channelType	= MDF_CHANNEL_TYPE_VALUE,
			.syncType		= MDF_SYNC_TYPE_NONE,
			.dataType		= MDF_DATA_TYPE_UNSIGNED_INTEL,
			.bitOffset		= 7,
			.byteOffset		= 10,
			.bitLength		= 1,
			.flags			= MDF_CN_FLAGS_BUS_EVENT
		},
		&(mdfCnLinkList_t)
		{
			.nameAddr	= mdfTxBlockWrite (mdf, "CAN_DataFrame.BRS"),
			.nextCnAddr	= addrDataBytes
		});

	uint64_t addrEsi = mdfCnBlockWrite (mdf,
		&(mdfCnDataSection_t)
		{
			.channelType	= MDF_CHANNEL_TYPE_VALUE,
			.syncType		= MDF_SYNC_TYPE_NONE,
			.dataType		= MDF_DATA_TYPE_UNSIGNED_INTEL,
			.bitOffset		= 6,
			.byteOffset		= 10,
			.bitLength		= 1,
			.flags			= MDF_CN_FLAGS_BUS_EVENT
		},
		&(mdfCnLinkList_t)
		{
			.nameAddr	= mdfTxBlockWrite (mdf, "CAN_DataFrame.ESI"),
			.nextCnAddr	= addrBrs
		});

	uint64_t addrEdl = mdfCnBlockWrite (mdf,
		&(mdfCnDataSection_t)
		{
			.channelType	= MDF_CHANNEL_TYPE_VALUE,
			.syncType		= MDF_SYNC_TYPE_NONE,
			.dataType		= MDF_DATA_TYPE_UNSIGNED_INTEL,
			.bitOffset		= 5,
			.byteOffset		= 10,
			.bitLength		= 1,
			.flags			= MDF_CN_FLAGS_BUS_EVENT
		},
		&(mdfCnLinkList_t)
		{
			.nameAddr	= mdfTxBlockWrite (mdf, "CAN_DataFrame.EDL"),
			.nextCnAddr	= addrEsi
		});

	uint64_t addrDir = mdfCnBlockWrite (mdf,
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
			.nameAddr	= mdfTxBlockWrite (mdf, "CAN_DataFrame.Dir"),
			.nextCnAddr	= addrEdl
		});

	uint64_t addrDataLength = mdfCnBlockWrite (mdf,
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
			.nameAddr	= mdfTxBlockWrite (mdf, "CAN_DataFrame.DataLength"),
			.nextCnAddr	= addrDir
		});

	uint64_t addrDlc = mdfCnBlockWrite (mdf,
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
			.nameAddr	= mdfTxBlockWrite (mdf, "CAN_DataFrame.DLC"),
			.nextCnAddr	= addrDataLength
		});

	uint64_t addrBusChannel = mdfCnBlockWrite (mdf,
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
			.nameAddr	= mdfTxBlockWrite (mdf, "CAN_DataFrame.BusChannel"),
			.nextCnAddr	= addrDlc
		});

	uint64_t addrIde = mdfCnBlockWrite (mdf,
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
			.nameAddr	= mdfTxBlockWrite (mdf, "CAN_DataFrame.IDE"),
			.nextCnAddr	= addrBusChannel
		});

	uint64_t addrId = mdfCnBlockWrite (mdf,
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
			.nameAddr	= mdfTxBlockWrite (mdf, "CAN_DataFrame.ID"),
			.nextCnAddr	= addrIde
		});

	// CAN_DataFrame Channel Group --------------------------------------------------------------------------------------------

	uint64_t addrCanDataFrame = mdfCnBlockWrite (mdf,
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
			.nameAddr		= mdfTxBlockWrite (mdf, "CAN_DataFrame"),
			.componentAddr	= addrId,
			.nextCnAddr		= 0,
		});

	uint64_t addrSi = mdfSiBlockWrite (mdf,
		&(mdfSiDataSection_t)
		{
			.sourceType	= MDF_SOURCE_TYPE_BUS,
			.busType	= MDF_BUS_TYPE_CAN
		},
		&(mdfSiLinkList_t)
		{
			.nameAddr		= mdfTxBlockWrite (mdf, "CAN"),
			.pathAddr		= mdfTxBlockWrite (mdf, "CAN"),
			.commentAddr	= mdfMdBlockWrite (mdf,
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
			"</SIcomment>")
		});

	mdfHdBlockLinkList (&hd)->firstFhAddr = mdfFhBlockWrite (mdf,
		&(mdfFhDataSection_t)
		{
			.unixTimeNs = timeLocal * 1e9
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

	mdfHdBlockLinkList (&hd)->commentAddr = mdfMdBlockWrite (mdf,
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

	uint64_t addrCc = mdfCcBlockWrite (mdf,
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
			.unitAddr = mdfTxBlockWrite (mdf, "s")
		});

	uint64_t addrTimestamp = mdfCnBlockWrite (mdf,
		&(mdfCnDataSection_t)
		{
			.channelType	= MDF_CHANNEL_TYPE_VALUE,
			.syncType		= MDF_SYNC_TYPE_NONE,
			.dataType		= MDF_DATA_TYPE_UNSIGNED_INTEL,
			.bitOffset		= 0,
			.byteOffset		= TIMESTAMP_BYTE_OFFSET,
			.bitLength		= TIMESTAMP_BIT_LENGTH,
			.flags			= MDF_CN_FLAGS_NONE
		},
		&(mdfCnLinkList_t)
		{
			.nameAddr		= mdfTxBlockWrite (mdf, "Timestamp"),
			.nextCnAddr		= addrCanDataFrame,
			.conversionAddr	= addrCc
		});

	uint64_t cgAddr = mdfCgBlockWrite (mdf,
		&(mdfCgDataSection_t)
		{
			.recordId		= 0x01,
			.flags			= MDF_CG_FLAGS_BUS_EVENT | MDF_CG_FLAGS_PLAIN_BUS_EVENT,
			.pathSeparator	= '.',
			.byteLength		= 19
		},
		&(mdfCgLinkList_t)
		{
			.firstCnAddr			= addrTimestamp,
			.acquisitionNameAddr	= mdfTxBlockWrite (mdf, "CAN_DataFrame"),
			.acquisitionSourceAddr	= addrSi
		});

	mdfBlock_t dg;
	mdfDgBlockInit (&dg,
		&(mdfDgLinkList_t)
		{
			.firstCgAddr = cgAddr
		});
	mdfHdBlockLinkList (&hd)->firstDgAddr = mdfBlockWrite (mdf, &dg);

	mdfDgBlockLinkList (&dg)->dataBlockAddr = mdfDtBlockWrite (mdf);

	// Rewrite Link Lists -----------------------------------------------------------------------------------------------------

	mdfRewriteBlockLinkList (mdf, &hd);
	mdfRewriteBlockLinkList (mdf, &dg);

	mdfBlockDealloc (&hd);
	mdfBlockDealloc (&dg);

	// Data Block Data Section ------------------------------------------------------------------------------------------------

	canFrame_t frame =
	{
		.id = 0x123,
		.data =
		{
			0x01,
			0x23,
			0x45,
			0x67,
			0x89,
			0xAB,
			0xCD,
			0xEF
		},
		.dlc = 8
	};

	writeCan1RxRecord (mdf, &frame, 0, 1);
	frame.id = 0x001;
	writeCan1RxRecord (mdf, &frame, 1000, 1);
	frame.id = 0x010;
	writeCan1RxRecord (mdf, &frame, 2000, 1);
	frame.id = 0x100;
	writeCan1RxRecord (mdf, &frame, 3000, 1);

	return 0;
}
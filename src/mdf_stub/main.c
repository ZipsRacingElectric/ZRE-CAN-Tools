// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "mdf/mdf_writer.h"
#include "debug.h"
#include "error_codes.h"

// C Standard Library
#include <errno.h>
#include <stdio.h>

void initTx (mdfBlock_t* tx, const char* text)
{
	size_t dataSectionSize = strlen (text) + 1;
	mdfBlockInit (tx, MDF_BLOCK_ID_TX, 0, dataSectionSize);
	memcpy (tx->dataSection, text, dataSectionSize);
}

void initMd (mdfBlock_t* md, const char* xml)
{
	size_t dataSectionSize = strlen (xml) + 1;
	mdfBlockInit (md, MDF_BLOCK_ID_MD, 0, dataSectionSize);
	memcpy (md->dataSection, xml, dataSectionSize);
}

void initCn (mdfBlock_t* cn, uint8_t byteOffset, uint8_t bitOffset, uint8_t bitLength, uint32_t b0_2, uint16_t b12_13)
{
	mdfBlockInit (cn, MDF_BLOCK_ID_CN, 8, 72);
	uint8_t* dataSection = cn->dataSection;

	dataSection [0] = b0_2;
	dataSection [1] = b0_2 >> 8;
	dataSection [2] = b0_2 >> 16;
	dataSection [3] = bitOffset;
	dataSection [4] = byteOffset;
	dataSection [8] = bitLength;
	dataSection [12] = b12_13;
	dataSection [13] = b12_13 >> 8;
}

uint64_t writeCn (FILE* mdf, uint64_t next, const char* text, uint8_t byteOffset, uint8_t bitOffset, uint8_t bitLength, uint32_t b0_2, uint16_t b12_13)
{
	mdfBlock_t cn;
	initCn (&cn, byteOffset, bitOffset, bitLength, b0_2, b12_13);
	mdfWriteBlock (mdf, &cn);

	mdfBlock_t tx;
	initTx (&tx, text);
	mdfWriteBlock (mdf, &tx);
	cn.linkList [0] = next;
	cn.linkList [2] = tx.addr;

	mdfRewriteBlockLinkList (mdf, &cn);
	return cn.addr;
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

	// Preamble ---------------------------------------------------------------------------------------------------------------

	mdfFileIdBlock_t fileIdBlock =
	{
		.data =
		{
			0x55, 0x6E, 0x46, 0x69, 0x6E, 0x4D, 0x46, 0x20,
			0x34, 0x2E, 0x31, 0x31, 0x20, 0x20, 0x20, 0x20,
			0x43, 0x45, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x9B, 0x01, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x25, 0x00, 0x00, 0x00,
		}
	};
	mdfWriteFileIdBlock (mdf, &fileIdBlock);

	mdfBlock_t hd;
	uint8_t hdDataSection [] =
	{
		0x00, 0x36, 0xD8, 0xCD, 0x44, 0xB5, 0x57, 0x18,
		0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	};
	mdfBlockInit (&hd, MDF_BLOCK_ID_HD, 6, sizeof (hdDataSection));
	memcpy (hd.dataSection, hdDataSection, sizeof (hdDataSection));

	mdfWriteBlock (mdf, &hd);

	mdfBlock_t txTimestamp;
	initTx (&txTimestamp, "Timestamp");
	mdfWriteBlock (mdf, &txTimestamp);

	mdfBlock_t cgTx;
	initTx (&cgTx, "CAN_DataFrame");
	mdfWriteBlock (mdf, &cgTx);

	mdfBlock_t dg;
	mdfBlockInit (&dg, MDF_BLOCK_ID_DG, 4, 8);
	*((uint64_t*) dg.dataSection) = 1;
	mdfWriteBlock (mdf, &dg);
	hd.linkList [0] = dg.addr;

	// Channel Group ----------------------------------------------------------------------------------------------------------

	// CG     : CAN_DataFrame
	// - CN   : Timestamp
	// - CN   : CAN_DataFrame
	//   - CN : CAN_DataFrame.ID
	//   - CN : CAN_DataFrame.DLC

	mdfBlock_t cgCanDataFrame;
	mdfBlockInit (&cgCanDataFrame, MDF_BLOCK_ID_CG, 6, 32);
	cgCanDataFrame.linkList [2] = cgTx.addr;

	// 01 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 06 00 2E 00 00 00 00 00 16 00 00 00 00 00 00 00
	((uint8_t*) cgCanDataFrame.dataSection) [0] = 0x01;
	((uint8_t*) cgCanDataFrame.dataSection) [16] = 0x06;
	((uint8_t*) cgCanDataFrame.dataSection) [18] = 0x2E;
	((uint8_t*) cgCanDataFrame.dataSection) [24] = 0x16;

	mdfWriteBlock (mdf, &cgCanDataFrame);
	dg.linkList [1] = cgCanDataFrame.addr;

	mdfBlock_t cnTimestamp;
	initCn (&cnTimestamp, 0, 0, 48, 0x000102, 0x0000);
	cnTimestamp.linkList [2] = txTimestamp.addr;
	mdfWriteBlock (mdf, &cnTimestamp);
	cgCanDataFrame.linkList [1] = cnTimestamp.addr;

	mdfBlock_t cnCanDataFrame;
	initCn (&cnCanDataFrame, 0, 0, 64, 0, 0);
	cnCanDataFrame.linkList [2] = cgTx.addr;
	mdfWriteBlock (mdf, &cnCanDataFrame);
	cnTimestamp.linkList [0] = cnCanDataFrame.addr;

	uint64_t addrBrs = writeCn (mdf, 0, "CAN_DataFrame.BRS", 12, 1, 1, 0x000000, 0x0400);
	uint64_t addrEsi = writeCn (mdf, addrBrs, "CAN_DataFrame.ESI", 12, 2, 1, 0x000000, 0x0400);
	uint64_t addrEdl = writeCn (mdf, addrEsi, "CAN_DataFrame.EDL", 12, 0, 1, 0x000000, 0x0400);
	uint64_t addrDir = writeCn (mdf, addrEdl, "CAN_DataFrame.Dir", 12, 0, 1, 0x000000, 0x0400);
	uint64_t addrDataBytes = writeCn (mdf, addrDir, "CAN_DataFrame.DataBytes", 14, 0, 64, 0x0A0000, 0x0400);
	uint64_t addrDataLength = writeCn (mdf, addrDataBytes, "CAN_DataFrame.DataLength", 7, 4, 4, 0x000000, 0x0400);
	uint64_t addrDlc = writeCn (mdf, addrDataLength, "CAN_DataFrame.DLC", 7, 4, 4, 0x000000, 0x0400);
	uint64_t addrIde = writeCn (mdf, addrDlc, "CAN_DataFrame.IDE", 8, 0, 1, 0x000000, 0x0400);
	uint64_t addrId = writeCn (mdf, addrIde, "CAN_DataFrame.ID", 8, 3, 29, 0x000000, 0x0400);
	uint64_t addrBusChannel = writeCn (mdf, addrId, "CAN_DataFrame.BusChannel", 8, 1, 2, 0x000000, 0x0408);
	cnCanDataFrame.linkList [1] = addrBusChannel;

	// mdfBlock_t si;
	// mdfBlockInit (&si, MDF_BLOCK_ID_SI, 3, 8);

	// // 02 02 00 00 00 00 00 00
	// ((uint8_t*) si.dataSection) [0] = 0x02;
	// ((uint8_t*) si.dataSection) [1] = 0x02;

	// mdfWriteBlock (mdf, &si);
	// cgCanDataFrame.linkList [2] = si.addr;

	mdfBlock_t fh;
	uint8_t fhDataSection [] =
	{
		0x00, 0x36, 0xD8, 0xCD, 0x44, 0xB5, 0x57, 0x18,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	};
	mdfBlockInit (&fh, MDF_BLOCK_ID_FH, 2, sizeof (fhDataSection));
	memcpy (fh.dataSection, fhDataSection, sizeof (fhDataSection));
	mdfWriteBlock (mdf, &fh);
	hd.linkList [1] = fh.addr;

	mdfBlock_t fhMd;
	initMd (&fhMd,
	"<FHcomment>\n"
	"	<TX>\n"
	"		Creation and logging of data.\n"
	"	</TX>\n"
	"	<tool_id>CE</tool_id>\n"
	"	<tool_vendor></tool_vendor>\n"
	"	<tool_version>01.04.02</tool_version>\n"
	"</FHcomment>");
	mdfWriteBlock (mdf, &fhMd);
	fh.linkList [1] = fhMd.addr;

	mdfBlock_t hdMd;
	initMd (&hdMd,
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
	mdfWriteBlock (mdf, &hdMd);
	hd.linkList [5] = hdMd.addr;

	// Link List --------------------------------------------------------------------------------------------------------------

	mdfRewriteBlockLinkList (mdf, &hd);
	mdfRewriteBlockLinkList (mdf, &dg);
	mdfRewriteBlockLinkList (mdf, &cgCanDataFrame);
	mdfRewriteBlockLinkList (mdf, &cnTimestamp);
	mdfRewriteBlockLinkList (mdf, &cnCanDataFrame);
	mdfRewriteBlockLinkList (mdf, &fh);

	return 0;
}
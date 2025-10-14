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

	mdfBlock_t tx;
	initTx (&tx, text);
	mdfWriteBlock (mdf, &tx);
	cn.linkList [0] = next;
	cn.linkList [2] = tx.addr;

	mdfWriteBlock (mdf, &cn);
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

	mdfFileIdBlock_t fileIdBlock;
	mdfWriteFileIdBlock (mdf, &fileIdBlock);

	mdfBlock_t hd;
	mdfBlockInit (&hd, MDF_BLOCK_ID_HD, 6, 32);
	mdfWriteBlock (mdf, &hd);

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

	// 01 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 06 00 2E 00 00 00 00 00 16 00 00 00 00 00 00 00
	((uint8_t*) cgCanDataFrame.dataSection) [0] = 0x01;
	((uint8_t*) cgCanDataFrame.dataSection) [16] = 0x06;
	((uint8_t*) cgCanDataFrame.dataSection) [18] = 0x2E;
	((uint8_t*) cgCanDataFrame.dataSection) [24] = 0x16;

	mdfWriteBlock (mdf, &cgCanDataFrame);
	dg.linkList [1] = cgCanDataFrame.addr;

	mdfBlock_t cgTx;
	initTx (&cgTx, "CAN_DataFrame");
	mdfWriteBlock (mdf, &cgTx);
	cgCanDataFrame.linkList [2] = cgTx.addr;

	mdfBlock_t cnTimestamp;
	initCn (&cnTimestamp, 0, 0, 48, 0x000102, 0x0000);
	mdfWriteBlock (mdf, &cnTimestamp);
	cgCanDataFrame.linkList [0] = cnTimestamp.addr;

	mdfBlock_t txTimestamp;
	initTx (&txTimestamp, "Timestamp");
	mdfWriteBlock (mdf, &txTimestamp);
	cnTimestamp.linkList [2] = txTimestamp.addr;

	mdfBlock_t cnCanDataFrame;
	initCn (&cnCanDataFrame, 0, 0, 64, 0, 0);
	cnCanDataFrame.linkList [2] = cgTx.addr;
	mdfWriteBlock (mdf, &cnCanDataFrame);
	cnTimestamp.linkList [0] = cnCanDataFrame.addr;

	uint64_t addrDir = writeCn (mdf, 0, "CAN_DataFrame.Dir", 12, 0, 1, 0x000000, 0x0400);
	uint64_t addrDataBytes = writeCn (mdf, addrDir, "CAN_DataFrame.DataBytes", 14, 0, 64, 0x0A0000, 0x0400);
	uint64_t addrDataLength = writeCn (mdf, addrDataBytes, "CAN_DataFrame.DataLength", 7, 4, 4, 0x000000, 0x0400);
	uint64_t addrDlc = writeCn (mdf, addrDataLength, "CAN_DataFrame.DLC", 7, 4, 4, 0x000000, 0x0400);
	uint64_t addrIde = writeCn (mdf, addrDlc, "CAN_DataFrame.IDE", 8, 0, 1, 0x000000, 0x0400);
	uint64_t addrId = writeCn (mdf, addrIde, "CAN_DataFrame.ID", 8, 3, 29, 0x000000, 0x0400);
	uint64_t addrBusChannel = writeCn (mdf, addrId, "CAN_DataFrame.BusChannel", 8, 1, 2, 0x000000, 0x0408);
	cnCanDataFrame.linkList [1] = addrBusChannel;

	// Link List --------------------------------------------------------------------------------------------------------------

	mdfRewriteBlockLinkList (mdf, &hd);
	mdfRewriteBlockLinkList (mdf, &dg);
	mdfRewriteBlockLinkList (mdf, &cgCanDataFrame);
	mdfRewriteBlockLinkList (mdf, &cnTimestamp);
	mdfRewriteBlockLinkList (mdf, &cnCanDataFrame);

	return 0;
}
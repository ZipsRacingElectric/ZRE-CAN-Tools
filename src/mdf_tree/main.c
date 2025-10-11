// MDF Tree -------------------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2025.10.09
//
// Description: Program for performing light parsing and interpretation of MDF files. Used to print the hierarchical structure
//   of an MDF file.

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "mdf/mdf_reader.h"
#include "debug.h"
#include "error_codes.h"
#include "list.h"

// C Standard Library
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Globals --------------------------------------------------------------------------------------------------------------------

listDefine (uint64_t);
list_t (uint64_t) addrHistory;

bool skipRepeats = false;

// Functions ------------------------------------------------------------------------------------------------------------------

struct treeArg
{
	struct treeArg* parent;
	size_t depth;
	bool last;
};
typedef struct treeArg treeArg_t;

void printTextBlockCharacter (mdfBlock_t* block, uint8_t data, void* arg)
{
	(void) block;
	FILE* stream = arg;

	if (data != '\0')
		fputc (data, stream);
}

int printTextBlock (mdfBlock_t* block, FILE* mdf, FILE* stream)
{
	fprintf (stream, " - ");

	if (mdfReadBlockDataSection (mdf, block, printTextBlockCharacter, stream))
	{
		int code = errno;
		fprintf (stderr, "Failed to read MDF block data section: %s.\n", errorMessage (code));
		return code;
	}

	return 0;
}

int printBlockTree (uint64_t addr, treeArg_t* arg, FILE* mdf, FILE* stream)
{
	for (size_t index = 0; index + 1 < arg->depth; ++index)
	{
		treeArg_t* a = arg->parent;
		for (size_t subIndex = index; subIndex + 2 < arg->depth; ++subIndex)
			a = a->parent;

		if (a->last)
			fprintf (stream, "   ");
		else
			fprintf (stream, "│  ");
	}

	if (arg->depth != 0)
	{
		if (arg->last)
			fprintf (stream, "└─ ");
		else
			fprintf (stream, "├─ ");
	}

	if (addr != 0)
	{
		mdfBlock_t block;
		fseek (mdf, addr, SEEK_SET);

		if (mdfReadBlockHeader (mdf, &block) != 0)
		{
			int code = errno;
			fprintf (stderr, "Failed to read MDF block header: %s.\n", errorMessage (code));
			return code;
		}

		if (mdfReadBlockLinkList (mdf, &block) != 0)
		{
			int code = errno;
			fprintf (stderr, "Failed to read MDF block link list: %s.\n", errorMessage (code));
			return code;
		}

		fprintf (stream, "%s - 0x%08lX", (char*) &block.header.blockId, block.addr);

		if (block.header.blockId == BLOCK_ID_TX)
			if (printTextBlock (&block, mdf, stream) != 0)
				return errno;

		if (skipRepeats && listContains (uint64_t) (&addrHistory, block.addr))
		{
			if (block.header.linkCount != 0)
				fprintf (stream, " (Repeat)");
			fprintf (stream, "\n");
			return 0;
		}

		fprintf (stream, "\n");

		listAppend (uint64_t) (&addrHistory, block.addr);

		for (size_t index = 0; index < block.header.linkCount; ++index)
		{
			treeArg_t childArg =
			{
				.parent	= arg,
				.depth	= arg->depth + 1,
				.last	= index == block.header.linkCount - 1
			};

			if (printBlockTree (block.linkList [index], &childArg, mdf, stream) != 0)
				return errno;
		}
	}
	else
	{
		fprintf (stream, "NULL\n");
	}

	return 0;
}

// Entrypoint -----------------------------------------------------------------------------------------------------------------

int main (int argc, char** argv)
{
	debugInit ();

	if (argc < 2)
	{
		fprintf (stderr, "Invalid arguments, usage: mdf-dump <MDF file path>.\n");
		return -1;
	}

	for (int index = 1; index < argc - 1; ++index)
	{
		if (strcmp (argv [index], "-r") == 0)
			skipRepeats = true;
	}

	listInit (uint64_t) (&addrHistory, 64);

	char* filePath = argv [argc - 1];

	mdfFileIdBlock_t fileIdBlock;
	FILE* mdf = mdfReaderOpen (filePath, &fileIdBlock);
	if (mdf == NULL)
	{
		int code = errno;
		fprintf (stderr, "Failed to open MDF file: %s.\n", errorMessage (code));
		return code;
	}

	treeArg_t arg =
	{
		.parent	= NULL,
		.depth	= 0,
		.last	= true
	};
	printBlockTree (sizeof (fileIdBlock), &arg, mdf, stdout);

	return 0;
}
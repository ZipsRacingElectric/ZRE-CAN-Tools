// Header
#include "mdf_block.h"

char* mdfBlockIdToString (uint64_t blockId)
{
	switch (blockId)
	{
	case MDF_BLOCK_ID_DT:
		return "##DT";
	case MDF_BLOCK_ID_MD:
		return "##MD";
	case MDF_BLOCK_ID_TX:
		return "##TX";
	}

	return "UNKNOWN";
}
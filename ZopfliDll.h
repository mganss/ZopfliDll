#ifndef _ZOPFLI_DLL_H
#define _ZOPFLI_DLL_H

#include "stdafx.h"
#include "zopfli/zopfli.h"
#include "CmdLine.h"

// compression level up to which do IIS gzip compression
#define MAX_IIS_GZIP_LEVEL 5

typedef struct CompressionContext
{
	ZopfliOptions options;
	BYTE *input_buffer = NULL;
	size_t input_buffer_size = 0;
	unsigned char *output_buffer = NULL;
	size_t output_buffer_size = 0;
	size_t output_used = 0;
	PVOID iis_compression_context;
	CmdLineContext cmd_line_context;
} CompressionContext;

#endif
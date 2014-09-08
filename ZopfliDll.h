#ifndef _ZOPFLI_DLL_H
#define _ZOPFLI_DLL_H

#include "stdafx.h"
#include "zopfli.h"
#include "CmdLine.h"

// compression level up to which do IIS gzip compression
#define MAX_IIS_GZIP_LEVEL 5

typedef struct CompressionContext
{
	ZopfliOptions options;
	BYTE *input_buffer;
	size_t input_buffer_size;
	unsigned char *output_buffer;
	size_t output_buffer_size;
	size_t output_used;
	PVOID iis_compression_context;
	CmdLineContext cmd_line_context;
} CompressionContext;

#endif
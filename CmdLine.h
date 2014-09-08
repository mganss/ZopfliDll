#ifndef _CMDLINE_H
#define _CMDLINE_H

typedef struct CmdLineContext
{
	HANDLE g_hChildStd_IN_Rd = NULL;
	HANDLE g_hChildStd_IN_Wr = NULL;
	HANDLE g_hChildStd_OUT_Rd = NULL;
	HANDLE g_hChildStd_OUT_Wr = NULL;
	bool child_created = false;
} CmdLineContext;

void ReadCmdLine();
HRESULT InitCmdLine();
void DeInitCmdLine();
HRESULT CreateCmdLine(CmdLineContext *context);
void DestroyCmdLine(CmdLineContext * context);
HRESULT WINAPI CmdLineCompress(
	IN  OUT PVOID           context,            // compression context
	IN      CONST BYTE *    input_buffer,       // input buffer
	IN      LONG            input_buffer_size,  // size of input buffer
	IN      PBYTE           output_buffer,      // output buffer
	IN      LONG            output_buffer_size, // size of output buffer
	OUT     PLONG           input_used,         // amount of input buffer used
	OUT     PLONG           output_used,        // amount of output buffer used
	IN      INT             compression_level   // compression level (1...10)
	);
extern bool cmd_line_configured;

#endif

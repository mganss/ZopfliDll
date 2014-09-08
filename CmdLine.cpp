#include <string>

#include "stdafx.h"
#include "ZopfliDll.h"
#include "CmdLine.h"

using namespace std;

#define BUFSIZE 4096 

static wstring cmd_line_prefix, cmd_line_suffix;
static bool cmd_line_read = false;
static int cmd_min = 0, cmd_max = 10;
bool cmd_line_configured = false;

HRESULT CreatePipes(CmdLineContext *context)
{
	SECURITY_ATTRIBUTES saAttr;

	// Set the bInheritHandle flag so pipe handles are inherited. 

	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle = TRUE;
	saAttr.lpSecurityDescriptor = NULL;

	// Create a pipe for the child process's STDOUT. 

	if (!CreatePipe(&context->g_hChildStd_OUT_Rd, &context->g_hChildStd_OUT_Wr, &saAttr, 0))
		return E_FAIL;

	// Ensure the read handle to the pipe for STDOUT is not inherited.

	if (!SetHandleInformation(context->g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0))
		return E_FAIL;

	// Create a pipe for the child process's STDIN. 

	if (!CreatePipe(&context->g_hChildStd_IN_Rd, &context->g_hChildStd_IN_Wr, &saAttr, 0))
		return E_FAIL;

	// Ensure the write handle to the pipe for STDIN is not inherited. 

	if (!SetHandleInformation(context->g_hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0))
		return E_FAIL;

	return S_OK;
}

HRESULT CreateChildProcess(CmdLineContext *context, int compression_level)
// Create a child process that uses the previously created pipes for STDIN and STDOUT.
{
	auto level = cmd_min + (int)(((double)(compression_level - (MAX_IIS_GZIP_LEVEL + 1)) / (10 - (MAX_IIS_GZIP_LEVEL + 1))) * (cmd_max - cmd_min));
	auto cmd_line = cmd_line_suffix.length() > 0 ? cmd_line_prefix + to_wstring(level) + cmd_line_suffix : cmd_line_prefix;
	PROCESS_INFORMATION piProcInfo;
	STARTUPINFO siStartInfo;
	BOOL bSuccess = FALSE;

	// Set up members of the PROCESS_INFORMATION structure. 

	ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

	// Set up members of the STARTUPINFO structure. 
	// This structure specifies the STDIN and STDOUT handles for redirection.

	ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
	siStartInfo.cb = sizeof(STARTUPINFO);
	//siStartInfo.hStdError = context->g_hChildStd_OUT_Wr;
	siStartInfo.hStdOutput = context->g_hChildStd_OUT_Wr;
	siStartInfo.hStdInput = context->g_hChildStd_IN_Rd;
	siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

	// Create the child process. 

	wchar_t cmd_line_modifiable[MAX_PATH];
	auto len = cmd_line._Copy_s(cmd_line_modifiable, MAX_PATH - 1, cmd_line.length());
	cmd_line_modifiable[len] = 0;
	bSuccess = CreateProcess(NULL,
		cmd_line_modifiable,     // command line 
		NULL,          // process security attributes 
		NULL,          // primary thread security attributes 
		TRUE,          // handles are inherited 
		0,             // creation flags 
		NULL,          // use parent's environment 
		NULL,          // use parent's current directory 
		&siStartInfo,  // STARTUPINFO pointer 
		&piProcInfo);  // receives PROCESS_INFORMATION 

	// If an error occurs, exit the application. 
	if (!bSuccess)
		return E_FAIL;
	else
	{
		// Close handles to the child process and its primary thread.
		// Some applications might keep these handles to monitor the status
		// of the child process, for example. 

		CloseHandle(piProcInfo.hProcess);
		CloseHandle(piProcInfo.hThread);

		CloseHandle(context->g_hChildStd_IN_Rd);
		context->g_hChildStd_IN_Rd = NULL;
		CloseHandle(context->g_hChildStd_OUT_Wr);
		context->g_hChildStd_OUT_Wr = NULL;
	}

	return S_OK;
}

void ReadCmdLine()
{
	if (cmd_line_read) return;

	cmd_line_read = true;

	// find dll path: http://stackoverflow.com/a/6924332
	wchar_t path[MAX_PATH];
	HMODULE hm = NULL;

	if (!GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
		GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
		(LPCWSTR)&ReadCmdLine,
		&hm)) return;

	auto module_len = GetModuleFileName(hm, path, MAX_PATH); 
	if (module_len == 0 || (module_len == MAX_PATH && GetLastError() == ERROR_INSUFFICIENT_BUFFER))
		return;
	
	// trim filename from end
	for (int fn = module_len - 1; fn > 0 && path[fn] != L'\\'; fn--) path[fn] = 0;
	wstring dll_path = wstring(path) + wstring(L"cmd.txt");

	FILE *cmd_file = NULL;
	if (_wfopen_s(&cmd_file, dll_path.c_str(), L"r") == 0 && cmd_file != NULL)
	{
		wchar_t raw_cmd[MAX_PATH];

		if (fgetws(raw_cmd, MAX_PATH, cmd_file) != NULL)
		{
			wchar_t expanded_cmd[MAX_PATH];
			auto path_len = ExpandEnvironmentStrings(raw_cmd, expanded_cmd, MAX_PATH - 1);
			if (path_len > 0 && path_len < MAX_PATH)
			{
				// trim end
				for (int ws = path_len - 2; ws > 0 && iswspace(expanded_cmd[ws]); ws--) expanded_cmd[ws] = 0;
				auto cmd_open_brace = wcsstr(expanded_cmd, L"{");
				if (cmd_open_brace != NULL)
				{
					swscanf_s(cmd_open_brace, L"{%d;%d}", &cmd_min, &cmd_max);
					auto cmd_close_brace = wcsstr(cmd_open_brace, L"}");
					if (cmd_close_brace != NULL)
					{
						cmd_line_prefix = wstring(expanded_cmd, cmd_open_brace - expanded_cmd);
						cmd_line_suffix = wstring(cmd_close_brace + 1);
						cmd_line_configured = true;
					}
				}
				else
				{
					cmd_line_prefix = wstring(expanded_cmd);
					cmd_line_configured = true;
				}
			}
		}

		fclose(cmd_file);
	}
}

HRESULT InitCmdLine()
{
	ReadCmdLine();
	return S_OK;
}

void DeInitCmdLine()
{
}

HRESULT CreateCmdLine(CmdLineContext * context)
{
	return S_OK;
}

void DestroyCmdLine(CmdLineContext * context)
{
	if (context->g_hChildStd_OUT_Rd != NULL)
	{
		CloseHandle(context->g_hChildStd_OUT_Rd);
		context->g_hChildStd_OUT_Rd = NULL;
	}
	if (context->g_hChildStd_OUT_Wr != NULL)
	{
		CloseHandle(context->g_hChildStd_OUT_Wr);
		context->g_hChildStd_OUT_Wr = NULL;
	}
	if (context->g_hChildStd_IN_Rd != NULL)
	{
		CloseHandle(context->g_hChildStd_IN_Rd);
		context->g_hChildStd_IN_Rd = NULL;
	}
	if (context->g_hChildStd_IN_Wr != NULL)
	{
		CloseHandle(context->g_hChildStd_IN_Wr);
		context->g_hChildStd_IN_Wr = NULL;
	}
}

HRESULT WINAPI CmdLineCompress(
	IN  OUT PVOID           context,            // compression context
	IN      CONST BYTE *    input_buffer,       // input buffer
	IN      LONG            input_buffer_size,  // size of input buffer
	IN      PBYTE           output_buffer,      // output buffer
	IN      LONG            output_buffer_size, // size of output buffer
	OUT     PLONG           input_used,         // amount of input buffer used
	OUT     PLONG           output_used,        // amount of output buffer used
	IN      INT             compression_level   // compression level (1...10)
	)
{
	auto cc = (CompressionContext *)context;

	*input_used = 0;
	*output_used = 0;

	if (!cc->cmd_line_context.child_created)
	{
		if (CreatePipes(&cc->cmd_line_context) != S_OK) return E_FAIL;
		if (CreateChildProcess(&cc->cmd_line_context, compression_level) != S_OK) return E_FAIL;
		cc->cmd_line_context.child_created = true;
	}

	if (input_buffer_size > 0)
	{
		DWORD in_used;
		if (!WriteFile(cc->cmd_line_context.g_hChildStd_IN_Wr, input_buffer, input_buffer_size, &in_used, NULL))
			return E_FAIL;

		*input_used = in_used;
	}
	else
	{
		if (cc->cmd_line_context.g_hChildStd_IN_Wr != NULL)
		{
			CloseHandle(cc->cmd_line_context.g_hChildStd_IN_Wr);
			cc->cmd_line_context.g_hChildStd_IN_Wr = NULL;
		}

		DWORD out_used;
		auto res = ReadFile(cc->cmd_line_context.g_hChildStd_OUT_Rd, output_buffer, output_buffer_size, &out_used, NULL);
		*output_used = out_used;
		if (res)
		{
			if (out_used == 0) // EOF -> done
			{
				CloseHandle(cc->cmd_line_context.g_hChildStd_OUT_Rd);
				cc->cmd_line_context.g_hChildStd_OUT_Rd = NULL;
				return S_FALSE;
			}
		}
		else
		{
			auto err = GetLastError();
			if (err == ERROR_BROKEN_PIPE) // EOF -> done
			{
				CloseHandle(cc->cmd_line_context.g_hChildStd_OUT_Rd);
				cc->cmd_line_context.g_hChildStd_OUT_Rd = NULL;
				return S_FALSE;
			}

			return E_FAIL;
		}
	}

	return S_OK;
}

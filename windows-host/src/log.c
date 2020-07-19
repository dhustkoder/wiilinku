#include <windows.h>
#include <stdio.h>
#include "log.h"

static char log_buffer[4096];
static HANDLE handles[2];

void log_internal_write(enum stdhandle_index idx, const char* fmtstr, ...)
{
	DWORD towrite, written;
	va_list valist;
	va_start(valist, fmtstr);
	towrite = vsnprintf(log_buffer, sizeof(log_buffer) - 1, fmtstr, valist);
	va_end(valist);
	
	log_buffer[towrite++] = '\n';
	
	const HANDLE stdhandle = handles[idx];
	WriteConsoleA(stdhandle, log_buffer, towrite, &written, NULL);
}


int log_init(void)
{
	AttachConsole(ATTACH_PARENT_PROCESS);
	handles[0] = GetStdHandle(STD_OUTPUT_HANDLE);
	handles[1] = GetStdHandle(STD_ERROR_HANDLE);
	log_info("\n");
	return 0;
}

void log_term(void)
{
	FreeConsole();
}


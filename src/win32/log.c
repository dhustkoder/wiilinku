#include <windows.h>
#include "log.h"

#define LOG_INTERNAL_BUFFER_SIZE (512)


static HANDLE stdout_handle;
static char log_internal_buffer[LOG_INTERNAL_BUFFER_SIZE];
static CRITICAL_SECTION log_internal_write_crit_sect;



void log_internal_write(enum log_level lvl, const char* fmt, ...)
{
	EnterCriticalSection(&log_internal_write_crit_sect); 

	int written;

	FMT_STR_VARGS_EX(log_internal_buffer, sizeof(log_internal_buffer), written, fmt, fmt);

	log_internal_buffer[written++] = '\n';
	WriteConsoleA(stdout_handle, log_internal_buffer, written, NULL, NULL);

	LeaveCriticalSection(&log_internal_write_crit_sect);
}


bool log_init(void)
{
	AttachConsole(ATTACH_PARENT_PROCESS);
	stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);
	WriteConsoleA(stdout_handle, "\n", 1, NULL, NULL);

	InitializeCriticalSectionAndSpinCount(
		&log_internal_write_crit_sect,
		~((DWORD)0)
	);

	return true;
}

void log_term(void)
{
	DeleteCriticalSection(&log_internal_write_crit_sect);
	FreeConsole();
}


#include <windows.h>
#include <stdint.h>
#include <limits.h>
#include <stdio.h>
#include "gui.h"
#include "connection.h"
#include "input.h"

#define LOG_IMPLEMENTATION
#define LOG_IMMEDIATE_MODE
#include "log.h"

static HANDLE stdout_handle;


static void log_buffer_flusher(const char* log_buffer, int size)
{
	WriteConsoleA(stdout_handle, log_buffer, size, NULL, NULL);
}


static bool init_platform(void)
{
	if (!log_init())
		return false;

	if (!connection_init())
		return false;

	if (!input_init())
		return false;

	if (!gui_init())
		return false;

	return true;
}

static void terminate_platform(void)
{
	gui_term();
	connection_term();
	input_term();
	log_term();
}



int CALLBACK WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	const int nCmdShow
)
{
	((void)hInstance);
	((void)hPrevInstance);
	((void)lpCmdLine);
	((void)lpCmdLine);
	((void)(nCmdShow));

	if (!init_platform())
		return EXIT_FAILURE;

	for (;;) {
		if (gui_update() == GUI_EVENT_WM_DESTROY)
			break;
	}

	terminate_platform();

	return EXIT_SUCCESS;
}

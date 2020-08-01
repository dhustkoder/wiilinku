#include <windows.h>
#include <stdint.h>
#include <limits.h>
#include <stdio.h>
#include "gui.h"
#include "connection.h"
#include "inputman.h"

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
	AttachConsole(ATTACH_PARENT_PROCESS);
	stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);
	WriteConsoleA(stdout_handle, "\n", 1, NULL, NULL);

	if (!log_init(log_buffer_flusher))
		return false;

	if (!inputman_init())
		return false;

	if (!connection_init(inputman_update))
		return false;

	if (!gui_init())
		return false;

	return true;
}

static void terminate_platform(void)
{
	gui_term();
	connection_term();
	inputman_term();
	log_term();
	FreeConsole();
}



int gui_main_thread(void)
{
	for (;;) {
		if (gui_win_update() == GUI_EVENT_WM_DESTROY)
			break;
	}

	return EXIT_SUCCESS;
}

int CALLBACK WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR lpCmdLine,
                     const int nCmdShow)
{
	((void)hInstance);
	((void)hPrevInstance);
	((void)lpCmdLine);
	((void)lpCmdLine);
	((void)(nCmdShow));

	if (!init_platform())
		return EXIT_FAILURE;

	int ret = gui_main_thread();


	terminate_platform();

	return ret;
}

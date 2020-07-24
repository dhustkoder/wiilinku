#include <stdint.h>
#include <limits.h>
#include <stdio.h>
#include "gui.h"
#include "x360emu.h"

#define NETN_IMPLEMENTATION
#include "netn.h"

#define LOG_IMPLEMENTATION
#include "log.h"

static HANDLE stdout_handle;

static void log_buffer_flusher(const char* log_buffer, int size)
{
	WriteConsoleA(stdout_handle, log_buffer, size, NULL, NULL);
}


static int init_platform(HINSTANCE hins, int ncmd)
{
	AttachConsole(ATTACH_PARENT_PROCESS);
	stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);

	if (log_init(log_buffer_flusher))
		return 1;

	log_info("\nLog Initialized!");

	if (gui_init(hins, ncmd))
		return 1;

	log_info("GUI Initialized");

	if (x360emu_init())
		return 1;

	log_info("x360emu Initialized");

	if (netn_init())
		return 1;

	log_info("Net Initialized");

	return 0;
}

static void terminate_platform(void)
{
	netn_term();
	x360emu_term();
	gui_term();
	log_term();
	FreeConsole();
}


static DWORD WINAPI jin_thread_main(LPVOID param)
{
	((void)param);
	struct netn_joy_packet jpkt;
	for (;;) {
		
		if (netn_joy_update(&jpkt))
			log_info("ERROR RECV INPUT");

		/*
		log_info(
			"GAMEPAD: %.8X %.4X %.4X %.4X %.4X\n"
			"WIIMOTE: %.8X",
			jpkt.gamepad.btns, 
			jpkt.gamepad.lsx, jpkt.gamepad.lsy, 
			jpkt.gamepad.rsx, jpkt.gamepad.rsy,
			jpkt.wiimote.btns
		);*/

		x360emu_update(&jpkt);
	}
}

int CALLBACK WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR lpCmdLine,
                     const int nCmdShow)
{
	((void)hPrevInstance);
	((void)lpCmdLine);

	if (init_platform(hInstance, nCmdShow))
		return 1;
/*
	DWORD thread_id;
	HANDLE jin_thread_handle = CreateThread(
		NULL,
		0,
		jin_thread_main,
		NULL,
		0,
		&thread_id
	);
*/
	for (;;) {
		if (gui_win_update())
			break;
		
		log_flush();
	}
/*
	if (!TerminateThread(jin_thread_handle, 0)) {
		log_info("failed to terminate thread: %d", GetLastError());
	}
*/
	terminate_platform();

	return 0;
}

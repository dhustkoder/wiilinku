#include <windows.h>
#include <stdint.h>
#include <limits.h>
#include <stdio.h>
#include "gui.h"
#include "x360emu.h"
#include "networking.h"

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

	const char* ip;
	unsigned short port;

	if (!log_init(log_buffer_flusher))
		return false;

	if (!networking_init(&ip, &port))
		return false;

	if (!x360emu_init())
		return false;

	if (!gui_init(ip, port))
		return false;

	return true;
}

static void terminate_platform(void)
{
	x360emu_term();
	gui_term();
	networking_term();
	log_term();
	FreeConsole();
}

struct input_packet input;
volatile int connected = 0;
volatile DWORD average_ms_input = 0;

static DWORD WINAPI networking_main_thread(LPVOID param)
{
	((void)param);
	

	/* input loop */

	DWORD tickcnt;
	DWORD lasttick;
	int input_frame_cnt = 0;

	for (;;) {

		while (!connected) {
			if (networking_try_handshake())
				connected = 1;
		}

		tickcnt = 0;

		while (connected) {
			lasttick = GetTickCount();

			if (!networking_input_update(&input)) {
				memset(&input, 0, sizeof input);
				connected = 0;
			}

			tickcnt += GetTickCount() - lasttick;
			if (++input_frame_cnt >= 60) {
				average_ms_input = tickcnt / input_frame_cnt;
				tickcnt = 0;
				input_frame_cnt = 0;
			}

			x360emu_update(&input);
		}

	}

	return EXIT_SUCCESS;
}

int gui_main_thread(void)
{
	for (;;) {
		if (gui_win_update() == GUI_EVENT_WM_DESTROY)
			break;
		log_info(
			"CONNECTED: %d\n"
			"GAMEPAD: %.8X %.4X %.4X %.4X %.4X\n"
			"WIIMOTE: %.8X\n"
			"AVERAGE INPUT FRAME TIME: %d (ms)",
			connected,
			input.gamepad.btns, 
			input.gamepad.lsx, input.gamepad.lsy, 
			input.gamepad.rsx, input.gamepad.rsy,
			input.wiimotes[0].btns,
			average_ms_input
		);
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


	DWORD thread_id;
	HANDLE joypads_thread_handle = CreateThread(
		NULL,
		0,
		networking_main_thread,
		NULL,
		0,
		&thread_id
	);

	int ret = gui_main_thread();

	if (!TerminateThread(joypads_thread_handle, 0)) {
		log_info("failed to terminate thread: %d", GetLastError());
	}

	terminate_platform();

	return ret;
}

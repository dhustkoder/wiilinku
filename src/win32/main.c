#include <windows.h>
#include <stdint.h>
#include <limits.h>
#include <stdio.h>
#include "gui.h"
#include "connection.h"
#include "input.h"
#include "log.h"


static volatile bool program_terminated = false;
static HANDLE connection_state_manager_thread_handle;
static HANDLE connection_input_manager_thread_handle;


static DWORD WINAPI connection_state_manager_thread(LPVOID lp)
{
	__pragma(warning(suppress:4100)) lp;

	DWORD ping_timer = GetTickCount();

	while (!program_terminated) {

		if (connection_is_connected()) {

			if ((GetTickCount() - ping_timer) >= 6000) {
				connection_ping_client();
				ping_timer = GetTickCount();
			}

		} else {

			connection_wait_client();

		}

		Sleep(1000);
	}

	return 0;
}

static DWORD WINAPI connection_input_manger_thread(LPVOID lp)
{
	__pragma(warning(suppress:4100)) lp;

	struct input_packet input;

	while (!program_terminated) {
		if (!connection_is_connected()) {
			Sleep(1000);
			continue;
		}

		if (connection_recv_input_packet(&input)) {
			input_update(&input);
		}
		
	}


	return 0;
}



static HANDLE* thandles[] = { 
	&connection_state_manager_thread_handle,
	&connection_input_manager_thread_handle
};
static LPTHREAD_START_ROUTINE troutines[] = {
	connection_state_manager_thread,
	connection_input_manger_thread
};

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

	for (int i = 0; i < sizeof(thandles)/sizeof(thandles[0]); ++i) {
		*thandles[i] = CreateThread(
			NULL,
			0,
			troutines[i],
			NULL,
			0,
			NULL
		);
		if (*thandles[i] == 0) {
			log_debug("Failed to start thread");
			return false;
		}
	}

	return true;
}

static void terminate_platform(void)
{
	program_terminated = true;

	for (int i = 0; i < sizeof(thandles)/sizeof(thandles[0]); ++i) {
		if (WaitForSingleObject(*thandles[i], 1000) != WAIT_OBJECT_0) {
			log_debug("wait thread termination failed, forcing termination");
			if (!TerminateThread(*thandles[i], 0)) {
				log_debug("forced termination failed");
			}
		}
	}

	gui_term();
	input_term();
	connection_term();
	log_term();
}



int CALLBACK WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	const int nCmdShow
)
{
	__pragma(warning(suppress:4100)) hInstance;
	__pragma(warning(suppress:4100)) hPrevInstance;
	__pragma(warning(suppress:4100)) lpCmdLine;
	__pragma(warning(suppress:4100)) lpCmdLine;
	__pragma(warning(suppress:4100)) nCmdShow;

	if (!init_platform())
		return EXIT_FAILURE;

	gui_event_t event = GUI_EVENT_NONE;

	int framecnt = 0;
	DWORD frametime;
	DWORD lasttick = GetTickCount();

	while (event != GUI_EVENT_WM_DESTROY) {
		frametime = GetTickCount();


		event = gui_update();
		++framecnt;

		if ((GetTickCount() - frametime) < 33)
			Sleep(33 - (GetTickCount() - frametime));

		if ((GetTickCount() - lasttick) >= 1000)  {
			log_info("MAIN THREAD FPS: %ld", framecnt);
			framecnt = 0;
			lasttick = GetTickCount();
		}
	}

	terminate_platform();

	return EXIT_SUCCESS;
}

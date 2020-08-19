#include <windows.h>
#include <stdint.h>
#include <limits.h>
#include <stdio.h>
#include "gui.h"
#include "connection.h"
#include "input.h"
#include "log.h"
#include "error.h"


static volatile bool terminate_threads = false;
static HANDLE connection_state_manager_thread_handle;
static HANDLE connection_input_manager_thread_handle;


static DWORD WINAPI connection_state_manager_thread(LPVOID lp)
{
	__pragma(warning(suppress:4100)) lp;
	

	while (!terminate_threads) {

		if (connection_is_connected()) {

			Sleep(PING_INTERVAL_SEC * 1000);

			if (!connection_ping_client()) {
				PlaySound(TEXT("DeviceDisconnect"), NULL, SND_ALIAS|SND_ASYNC);
				gui_set_client_ip_string(NULL);
			}


		} else {

			if (connection_wait_client()) {
				PlaySound(TEXT("DeviceConnect"), NULL, SND_ALIAS|SND_ASYNC);
				gui_set_client_ip_string(connection_get_client_address());
			}

		}
		
	}

	return 0;
}

static DWORD WINAPI connection_input_manger_thread(LPVOID lp)
{
	__pragma(warning(suppress:4100)) lp;

	struct input_packet input;

	while (!terminate_threads) {
		if (!connection_is_connected()) {
			gui_set_connected_controllers(0);
			Sleep(1000);
			continue;
		}
		
		if (connection_recv_input_packet(&input)) {
			gui_set_connected_controllers(input.flags);
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

	gui_set_local_ip_string(connection_get_host_address());

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
	terminate_threads = true;

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

	if (!init_platform()) {
		MessageBox(
		  NULL,
		  get_last_error(),
		  "WiiLinkU Error",
		  MB_ICONERROR
		);
		return EXIT_FAILURE;
	}

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
			framecnt = 0;
			lasttick = GetTickCount();
		}
	}

	terminate_platform();

	return EXIT_SUCCESS;
}

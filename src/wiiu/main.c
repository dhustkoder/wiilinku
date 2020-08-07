#include <wut_types.h>
#include <whb/proc.h>
#include <whb/libmanager.h>
#include <coreinit/thread.h>
#include <stdlib.h>
#include <string.h>
#include "connection.h"
#include "input.h"
#include "video.h"


static const char* logo_ascii =
" __        __  _   _   _       _           _      _   _      \n"
" \\ \\      / / (_) (_) | |     (_)  _ __   | | __ | | | |   \n"
"  \\ \\ /\\ / /  | | | | | |     | | | '_ \\  | |/ / | | | | \n"
"   \\ V  V /   | | | | | |___  | | | | | | |   <  | |_| |    \n"
"    \\_/\\_/    |_| |_| |_____| |_| |_| |_| |_|\\_\\  \\___/ \n"
"                                                             \n";
                                                                       


static volatile bool terminate_threads = false;


static int connection_state_manager_thread(
	__attribute__((unused)) int  a,
	__attribute__((unused)) char** b
)
{

	OSTick ping_timer = OSGetSystemTick();

	while (!terminate_threads) {

		if (connection_is_connected()) {

			if (OSTicksToSeconds((ping_timer - OSGetSystemTick())) >= PING_INTERVAL_SEC) {
				if (!connection_ping_host()) {
					video_log_printf("disconnected");
				}
				ping_timer = OSGetSystemTick();
			} else {
				OSSleepTicks(OSSecondsToTicks(PING_INTERVAL_SEC));
			}

		} else {

			if (connection_connect(HOST_IP_ADDRESS)) {
				video_log_printf("connected");
			} else {
				video_log_printf("failed to connect");
			}

			OSSleepTicks(OSMillisecondsToTicks(1000));

		}
	}

	return 0;
}


static int input_manager_thread(
	__attribute__((unused)) int  a,
	__attribute__((unused)) char** b
)
{
	struct input_feedback_packet feedback;
	struct input_packet input;

	while (!terminate_threads) {
		input_update();
		input_fetch(&input);

		connection_send_input_packet(&input);

		if (connection_receive_input_feedback_packet(&feedback)) {
			input_update_feedback(&feedback);
		}

		OSSleepTicks(OSMillisecondsToTicks(6));
	}

	return 0;
}


static bool platform_init(void)
{
	WHBProcInit();
	input_init();

	if (!video_init())
		return false;

	if (!connection_init())
		return false;



	OSRunThread(
		OSGetDefaultThread(0),
		(OSThreadEntryPointFn)input_manager_thread,
		0,
		NULL
	);

	OSRunThread(
		OSGetDefaultThread(2),
		(OSThreadEntryPointFn)connection_state_manager_thread,
		0,
		NULL
	);

	return true;
}


static void platform_term(void)
{
	terminate_threads = true;

	connection_term();
	
	video_term();

	input_term();

	WHBProcShutdown();
}


int main(void)
{
	if (!platform_init())
		return EXIT_FAILURE;
	
	struct input_packet input;
	memset(&input, 0, sizeof input);

	for (;;) {
		input_fetch(&input);

		if (input.gamepad.btns&WIIU_GAMEPAD_BTN_HOME || input.wiimotes[0].btns&WIIMOTE_BTN_HOME)
			break;

		video_render_clear();
		video_render_text(0, 0, logo_ascii);
		video_render_text_aligned_fmt(35, 10,
			"GAMEPAD BTNS: %.8X\n"
			"WIIMOTE[0] BTNS: %.8X\n",
			input.gamepad.btns,
			input.wiimotes[0].btns
		);
		video_render_flip();
	}

	platform_term();

	return EXIT_SUCCESS;
}

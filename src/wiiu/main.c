#include <wut_types.h>
#include <whb/proc.h>
#include <whb/libmanager.h>
#include <coreinit/thread.h>
#include <stdlib.h>
#include <string.h>
#include "connection.h"
#include "input.h"
#include "video.h"
#include "utils.h"


static const char* logo_ascii =
" __        __  _   _   _       _           _      _   _      \n"
" \\ \\      / / (_) (_) | |     (_)  _ __   | | __ | | | |   \n"
"  \\ \\ /\\ / /  | | | | | |     | | | '_ \\  | |/ / | | | | \n"
"   \\ V  V /   | | | | | |___  | | | | | | |   <  | |_| |    \n"
"    \\_/\\_/    |_| |_| |_____| |_| |_| |_| |_|\\_\\  \\___/ \n"
"                                            "WLU_VERSION_STR"\n";
                                                                       


static volatile bool terminate_threads = false;
static volatile const char* entered_ip = NULL;


static int connection_state_manager_thread(
	__attribute__((unused)) int  a,
	__attribute__((unused)) const char** b
)
{
	while (!terminate_threads) {

		if (connection_is_connected()) {
			
			OSSleepTicks(OSSecondsToTicks(PING_INTERVAL_SEC));

			if (!connection_ping_host()) {
				video_log_printf("disconnected");
			}

		} else {

			if (entered_ip != NULL) {
				video_log_printf("trying to connect...");
				if (connection_connect((char*)entered_ip)) {
					video_log_printf("connected");
				} else {
					video_log_printf("failed to connect");
					entered_ip = NULL;
				}
			}

			OSSleepTicks(OSMillisecondsToTicks(1000));

		}
	}

	return 0;
}


static int input_manager_thread(
	__attribute__((unused)) int  a,
	__attribute__((unused)) const char** b
)
{
	struct input_feedback_packet feedback;
	struct input_packet input;

	while (!terminate_threads) {
		input_update(&input);

		if (connection_is_connected()) {
			connection_send_input_packet(&input);

			if (connection_receive_input_feedback_packet(&feedback)) {
				input_update_feedback(&feedback);
			}
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

	#ifdef WLU_DEBUG
	video_log_printf("you are running a debug build");
	#else
	video_log_printf("you are running a release build");
	#endif

	OSRunThread(
		OSGetDefaultThread(0),
		input_manager_thread,
		0,
		NULL
	);

	OSRunThread(
		OSGetDefaultThread(2),
		connection_state_manager_thread,
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
	
	char ipbuf[24] = "192.168.000.000\0";
	const int ip_cur_pos_table[12] = { 0, 1, 2, 4, 5, 6, 8, 9, 10, 12, 13, 14 };
	int ip_cur_pos = 0;

	VPADStatus vpad;
	memset(&vpad, 0, sizeof(VPADStatus));

	for (;;) {
		input_fetch_vpad(&vpad);

		if (vpad.trigger&VPAD_BUTTON_HOME)
			break;

		if (!connection_is_connected()) {
			if (vpad.trigger&VPAD_BUTTON_RIGHT && ip_cur_pos < 11) {
				++ip_cur_pos;
			} else if (vpad.trigger&VPAD_BUTTON_LEFT && ip_cur_pos > 0) {
				--ip_cur_pos;
			}

			if (vpad.trigger&VPAD_BUTTON_UP && ipbuf[ip_cur_pos_table[ip_cur_pos]] < '9') {
				++ipbuf[ip_cur_pos_table[ip_cur_pos]];
			} else if (vpad.trigger&VPAD_BUTTON_DOWN && ipbuf[ip_cur_pos_table[ip_cur_pos]] > '0') {
				--ipbuf[ip_cur_pos_table[ip_cur_pos]];
			}

			if (vpad.trigger&VPAD_BUTTON_PLUS) {
				entered_ip = ipbuf;
			}
		}

		video_render_clear();
		video_render_text(0, 0, logo_ascii);

		
		video_render_text_fmt(0, 6, "Enter IP: %s", ipbuf);
		video_render_text(10 + ip_cur_pos_table[ip_cur_pos], 7, "^");
		video_render_text_aligned(0, 8,
			"EXIT      = HOME\n"
			"CONNECT   = +\n"
			"IP SELECT = DPAD LEFT RIGHT UP DOWN\n"
		);

		video_render_text_aligned_fmt(45, 8,
			" -- GAMEPAD --\n"
			"BTNS: %.8X\n"
			"RSX: %.2f\n"
			"RSY: %.2f\n"
			"LSX: %.2f\n"
			"LSY: %.2f\n",
			vpad.hold,
			vpad.rightStick.x,
			vpad.rightStick.y,
			vpad.leftStick.x,
			vpad.leftStick.y
		);

		video_render_flip();
	}

	platform_term();

	return EXIT_SUCCESS;
}

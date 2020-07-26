#include <wut_types.h>
#include <whb/proc.h>
#include <whb/libmanager.h>
#include <coreinit/thread.h>
#include <stdlib.h>
#include "networking.h"
#include "inputman.h"
#include "video.h"


#define LOG_IMPLEMENTATION
#define LOG_MAX_LINES (6)
#include "log.h"


// screen

static char input_log[512];

static const char* logo_ascii =
	"          _ _                       \n"
	"          _ _                        \n"
	"         (_(_)                       \n"
	"__      ___ _ _   _ _ __   _____  __ \n"
	"\\ \\ /\\ / | | | | | | '_ \\ / __\\ \\/ / \n"
	" \\ V  V /| | | |_| | |_) | (__ >  <  \n"
	"  \\_/\\_/ |_|_|\\__,_| .__/ \\___/_/\\_\\ \n"
	"                   | |               \n"
	"                   |_|               \n";



static void log_buffer_flusher(const char* buf, int len)
{
	video_render_text(0, 13, buf);
}

static bool platform_init(void)
{
	WHBProcInit();
	WHBInitializeSocketLibrary();
	inputman_init();

	if (!log_init(log_buffer_flusher))
		return false;

	if (!video_init())
		return false;

	if (!networking_init())
		return false;

	return true;
}


static void platform_term(void)
{

	networking_term();

	video_term();

	log_term();

	inputman_term();

	WHBDeinitializeSocketLibrary();

	WHBProcShutdown();
}

volatile int connection_status = 0;
volatile int program_termination_requested = 0;



static int gui_main_thread(void)
{

	struct input_packet input;

	int connected = 0;

	for (;;) {
		video_render_clear();
		video_render_text(0, 0, logo_ascii);
		video_render_text_aligned(35, 10, input_log);

		if (!connected) {
			if (networking_try_handshake("192.168.15.7", 7173))
				connected = 1;
		}

		inputman_update(&input);

		sprintf(
			input_log,
			"CONNECTED: %d\n"
			"GAMEPAD BTNS: %.8X\n"
			"WIIMOTE[0] BTNS: %.8X\n",
			connected,
			input.gamepad.btns,
			input.wiimotes[0].btns
		);

		if (input.gamepad.btns&WIIU_GAMEPAD_BTN_HOME)
			break;

		if (connected) {
			if (!networking_input_update(&input))
				connected = 0;
			else
				log_info("send success");
		}

		log_flush();
		video_render_flip();
	}

	return EXIT_SUCCESS;
} 


int main(void)
{
	if (!platform_init())
		return EXIT_FAILURE;
	
	// OSRunThread(OSGetDefaultThread(0), networking_main_thread, 0, NULL);
	
	int ret = gui_main_thread();

	platform_term();

	return ret;
}

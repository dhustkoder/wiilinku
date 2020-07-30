#include <wut_types.h>
#include <whb/proc.h>
#include <whb/libmanager.h>
#include <coreinit/thread.h>
#include <stdlib.h>
#include "connection.h"
#include "inputman.h"
#include "video.h"


#define LOG_IMPLEMENTATION
#define LOG_MAX_LINES (3)
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

	if (!connection_init())
		return false;

	return true;
}


static void platform_term(void)
{

	connection_term();

	video_term();

	log_term();

	inputman_term();

	WHBDeinitializeSocketLibrary();

	WHBProcShutdown();
}


static int gui_main_thread(void)
{

	if (!connection_connect("192.168.15.7", 7173))
		return EXIT_FAILURE;

	log_debug("connected");

	struct input_packet input;

	for (;;) {
		inputman_fetch(&input);

		if (input.gamepad.btns&WIIU_GAMEPAD_BTN_HOME)
			break;

		sprintf(
			input_log,
			"GAMEPAD BTNS: %.8X\n"
			"WIIMOTE[0] BTNS: %.8X\n",
			input.gamepad.btns,
			input.wiimotes[0].btns
		);

		video_render_clear();
		video_render_text(0, 0, logo_ascii);
		video_render_text_aligned(35, 10, input_log);
		log_flush();
		video_render_flip();
	}

	return EXIT_SUCCESS;
} 


int main(void)
{
	if (!platform_init())
		return EXIT_FAILURE;
	
	int ret = gui_main_thread();

	platform_term();

	return ret;
}

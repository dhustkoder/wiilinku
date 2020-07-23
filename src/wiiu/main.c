#include <stdio.h>
#include <malloc.h>
#include <wut_types.h>
#include <whb/proc.h>
#include <coreinit/screen.h>
#include <padscore/kpad.h>
#include <vpad/input.h>
#include <nn/ac/ac_c.h>
#include <whb/libmanager.h>
#include <coreinit/memheap.h>
#include <coreinit/cache.h>
#include <coreinit/memfrmheap.h>
#include <coreinit/thread.h>


#define NETN_IMPLEMENTATION
#include "netn.h"


#define LOG_IMPLEMENTATION
#define LOG_MAX_LINES (6)
#include "log.h"


// screen
static MEMHeapHandle heap;
static uint32_t bufsz_tv;
static uint32_t bufsz_drc;
static unsigned char* buf_tv;
static unsigned char* buf_drc;
static char input_log[256];

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



static int video_init(void)
{
	// Init screen and screen buffers
	heap = MEMGetBaseHeapHandle(MEM_BASE_HEAP_MEM1);
	MEMRecordStateForFrmHeap(heap, MEM_FRAME_HEAP_TAG);
	OSScreenInit();
	bufsz_tv = OSScreenGetBufferSizeEx(SCREEN_TV);
	bufsz_drc = OSScreenGetBufferSizeEx(SCREEN_DRC);
	buf_tv  = MEMAllocFromFrmHeapEx(heap, bufsz_tv, 4);
	buf_drc = MEMAllocFromFrmHeapEx(heap, bufsz_drc, 4);
	OSScreenSetBufferEx(SCREEN_TV, buf_tv);
	OSScreenSetBufferEx(SCREEN_DRC, buf_drc);
	OSScreenEnableEx(SCREEN_TV, 1);
	OSScreenEnableEx(SCREEN_DRC, 1);

	// Clear screens
	OSScreenClearBufferEx(SCREEN_TV, 0x000000FF);
	OSScreenClearBufferEx(SCREEN_DRC, 0x000000FF);

	return 0;
}

static void video_term(void)
{
	OSScreenShutdown();
	MEMFreeByStateToFrmHeap(heap, MEM_FRAME_HEAP_TAG);
}

static void video_render_clear(void)
{
	OSScreenClearBufferEx(SCREEN_TV, 0x000000FF);
	OSScreenClearBufferEx(SCREEN_DRC, 0x000000FF);
}

static void video_render_flip(void)
{
	DCFlushRange(buf_tv, bufsz_tv);
	DCFlushRange(buf_drc, bufsz_drc);
	OSScreenFlipBuffersEx(SCREEN_TV);
	OSScreenFlipBuffersEx(SCREEN_DRC);
}

static void video_render_text(int x, int y, const char* buf)
{
	OSScreenPutFontEx(SCREEN_TV, x, y, buf);
	OSScreenPutFontEx(SCREEN_DRC, x, y, buf);
}

static void video_render_text_aligned(int x, int y, char* buf)
{
	char* nwidx;
	while ((nwidx = strchr(buf, '\n')) != NULL) {
		*nwidx = '\0';
		video_render_text(x, y++, buf);
		*nwidx = '\n';
		buf = nwidx + 1;
	}

	if (*buf != '\0')
		video_render_text(x, y, buf);
}

static void log_buffer_flusher(const char* buf)
{
	video_render_text(0, 10, buf);
}

static int platform_init(void)
{
	WHBProcInit();
	WHBInitializeSocketLibrary();
	VPADInit();
	KPADInit();
	WPADEnableURCC(1);

	if (log_init(log_buffer_flusher))
		return 1;

	log_info("log initialized");

	if (video_init())
		return 1;

	log_info("video initialized");

	if (netn_init())
		return 1;

	log_info("net initialized");

	return 0;
}


static void platform_term(void)
{

	netn_term();

	video_term();

	log_term();

	WHBDeinitializeSocketLibrary();

	WHBProcShutdown();
}


int main(int argc, char **argv)
{

	platform_init();

	// Gamepad key state data
	VPADReadError verror;
	int32_t kerror;
	VPADStatus vpad_data;
	KPADStatus kpad_data;

	memset(&vpad_data, 0, sizeof vpad_data);
	memset(&kpad_data, 0, sizeof kpad_data);


	for (;;) {
		VPADRead(VPAD_CHAN_0, &vpad_data, 1, &verror);
		KPADReadEx(WPAD_CHAN_0, &kpad_data, 1, &kerror);

		video_render_clear();

		video_render_text(0, 0, logo_ascii);

		video_render_text_aligned(20, 10, input_log);


		struct netn_joy_packet jpkt;
		
		const VPADVec2D ls = vpad_data.leftStick;
		const VPADVec2D rs = vpad_data.rightStick;
		
		VPADVec3D gyro = vpad_data.gyro;
		if (gyro.x > 0.04 || gyro.x < -0.04)
			gyro.x *= -2.15;
		else
			gyro.x = 0;
		if (gyro.y > 0.04 || gyro.y < -0.04)
			gyro.y *= -2.15;
		else
			gyro.y = 0;

		#define CLAMPF(n, max, min) ((n) > (max)) ? (max) : ((n) < (min)) ? (min) : (n)
		
		jpkt.gamepad.btns = vpad_data.hold;
		jpkt.gamepad.lsx = CLAMPF((ls.x + gyro.y), 1.0, -1.0) * INT16_MAX;
		jpkt.gamepad.lsy = CLAMPF((ls.y + gyro.x), 1.0, -1.0) * INT16_MAX;
		jpkt.gamepad.rsx = rs.x * INT16_MAX;
		jpkt.gamepad.rsy = rs.y * INT16_MAX;
		jpkt.wiimote.btns = kpad_data.hold;
		
		sprintf(
			input_log,
			"GAMEPAD: %.8X %.4X %.4X %.4X %.4X\n"
			"GAMEPAD GYRO: %.2f %.2f %.2f\n"
			"WIIMOTE: %.8X\n",
			jpkt.gamepad.btns,
			jpkt.gamepad.lsx,
			jpkt.gamepad.lsy,
			jpkt.gamepad.rsx,
			jpkt.gamepad.rsy,
			gyro.x, gyro.y, gyro.z,
			jpkt.wiimote.btns
		);

		netn_joy_update(&jpkt);

		if (vpad_data.trigger & VPAD_BUTTON_HOME || kpad_data.trigger & WPAD_BUTTON_HOME)
			break;


		log_flush();
		video_render_flip();

	}

	platform_term();

	return 0;
}

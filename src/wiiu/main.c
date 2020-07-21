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

// screen
static MEMHeapHandle heap;
static uint32_t bufsz_tv;
static uint32_t bufsz_drc;
static unsigned char* buf_tv;
static unsigned char* buf_drc;
static char log_buffer[16000];
static int log_buffer_idx = 0;



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


	memset(log_buffer, 0, sizeof log_buffer);

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
	OSScreenPutFontEx(SCREEN_TV, 0, 0, log_buffer);
	OSScreenPutFontEx(SCREEN_DRC, 0, 0, log_buffer);
	log_buffer_idx = 0;

	DCFlushRange(buf_tv, bufsz_tv);
	DCFlushRange(buf_drc, bufsz_drc);
	OSScreenFlipBuffersEx(SCREEN_TV);
	OSScreenFlipBuffersEx(SCREEN_DRC);
}

static void video_printf(const char* fmt, ...)
{
	va_list valist;
	va_start(valist, fmt);
	log_buffer_idx += vsprintf(log_buffer + log_buffer_idx, fmt, valist);
	va_end(valist);
}

static int platform_init(void)
{
	WHBProcInit();
	WHBInitializeSocketLibrary();
	VPADInit();
	KPADInit();
	WPADEnableURCC(1);

	if (video_init())
		return 1;

	if (netn_init())
		return 1;

	return 0;
}


static void platform_term(void)
{

	netn_term();

	video_term();

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

		video_printf(
			"          _ _                       \n"
			"          _ _                        \n"
			"         (_(_)                       \n"
			"__      ___ _ _   _ _ __   _____  __ \n"
			"\\ \\ /\\ / | | | | | | '_ \\ / __\\ \\/ / \n"
			" \\ V  V /| | | |_| | |_) | (__ >  <  \n"
			"  \\_/\\_/ |_|_|\\__,_| .__/ \\___/_/\\_\\ \n"
			"                   | |               \n"
			"                   |_|               \n"
 		);

		video_printf("Connected.\n");


		struct netn_joy_packet jpkt;
		
		const VPADVec2D ls = vpad_data.leftStick;
		const VPADVec2D rs = vpad_data.rightStick;
		
		jpkt.gamepad.btns = vpad_data.hold;
		jpkt.gamepad.lsx = ls.x * INT16_MAX;
		jpkt.gamepad.lsy = ls.y * INT16_MAX;
		jpkt.gamepad.rsx = rs.x * INT16_MAX;
		jpkt.gamepad.rsy = rs.y * INT16_MAX;

		jpkt.wiimote.btns = kpad_data.hold;
		
		video_printf(
			"GAMEPAD: %.8X %.4X %.4X %.4X %.4X\n"
			"WIIMOTE: %.8X\n",
			jpkt.gamepad.btns,
			jpkt.gamepad.lsx,
			jpkt.gamepad.lsy,
			jpkt.gamepad.rsx,
			jpkt.gamepad.rsy,
			jpkt.wiimote.btns
		);

		netn_joy_update(&jpkt);

		if (vpad_data.trigger & VPAD_BUTTON_HOME || kpad_data.trigger & WPAD_BUTTON_HOME)
			break;

		video_render_flip();

	}

	platform_term();

	return 0;
}

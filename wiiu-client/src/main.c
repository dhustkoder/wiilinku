#include <stdio.h>
#include <malloc.h>
#include <wut_types.h>
#include <whb/proc.h>
#include <coreinit/screen.h>
#include <padscore/kpad.h>
#include <vpad/input.h>
#include <nsysnet/socket.h>
#include <nn/ac/ac_c.h>
#include <whb/libmanager.h>
#include <coreinit/memheap.h>
#include <coreinit/cache.h>
#include <coreinit/memfrmheap.h>
#include <coreinit/thread.h>
#include "udp.h"

#define FRAME_HEAP_TAG (0x000DECAF)

enum xusb_gamepad_button {
    XUSB_GAMEPAD_DPAD_UP            = 0x0001,
    XUSB_GAMEPAD_DPAD_DOWN          = 0x0002,
    XUSB_GAMEPAD_DPAD_LEFT          = 0x0004,
    XUSB_GAMEPAD_DPAD_RIGHT         = 0x0008,
    XUSB_GAMEPAD_START              = 0x0010,
    XUSB_GAMEPAD_BACK               = 0x0020,
    XUSB_GAMEPAD_LEFT_THUMB         = 0x0040,
    XUSB_GAMEPAD_RIGHT_THUMB        = 0x0080,
    XUSB_GAMEPAD_LEFT_SHOULDER      = 0x0100,
    XUSB_GAMEPAD_RIGHT_SHOULDER     = 0x0200,
    XUSB_GAMEPAD_GUIDE              = 0x0400,
    XUSB_GAMEPAD_A                  = 0x1000,
    XUSB_GAMEPAD_B                  = 0x2000,
    XUSB_GAMEPAD_X                  = 0x4000,
    XUSB_GAMEPAD_Y                  = 0x8000
};

char send_buffer[32];

int main(int argc, char **argv)
{
	WHBProcInit();
	WHBInitializeSocketLibrary();
	VPADInit();
	KPADInit();
	WPADEnableURCC(1);

	// Init screen and screen buffers
	MEMHeapHandle heap = MEMGetBaseHeapHandle(MEM_BASE_HEAP_MEM1);
	MEMRecordStateForFrmHeap(heap, FRAME_HEAP_TAG);
	OSScreenInit();
	const uint32_t sBufferSizeTV = OSScreenGetBufferSizeEx(SCREEN_TV);
	const uint32_t sBufferSizeDRC = OSScreenGetBufferSizeEx(SCREEN_DRC);
	void *ScreenBuffer0 = MEMAllocFromFrmHeapEx(heap, sBufferSizeTV, 4);
	void *ScreenBuffer1 = MEMAllocFromFrmHeapEx(heap, sBufferSizeDRC, 4);
	OSScreenSetBufferEx(SCREEN_TV, ScreenBuffer0);
	OSScreenSetBufferEx(SCREEN_DRC, ScreenBuffer1);
	OSScreenEnableEx(SCREEN_TV, 1);
	OSScreenEnableEx(SCREEN_DRC, 1);

	// Clear screens
	OSScreenClearBufferEx(SCREEN_TV, 0x000000FF);
	OSScreenClearBufferEx(SCREEN_DRC, 0x000000FF);

	// Gamepad key state data
	VPADReadError error;
	VPADStatus vpad_data;

	int connected = 0;

	for (;;) {
		VPADRead(VPAD_CHAN_0, &vpad_data, 1, &error);

		OSScreenClearBufferEx(SCREEN_TV, 0x000000FF);
		OSScreenClearBufferEx(SCREEN_DRC, 0x000000FF);
		OSScreenPutFontEx(SCREEN_DRC, -4, 0, "!Hello World!");

		if (connected) {
			OSScreenPutFontEx(SCREEN_DRC, -4, 1, "Connected.");
			OSScreenPutFontEx(SCREEN_DRC, -4, 2, send_buffer);
			uint16_t buttons = 0x00;

			if (vpad_data.hold & VPAD_BUTTON_A)
				buttons |= XUSB_GAMEPAD_A;
			if (vpad_data.hold & VPAD_BUTTON_B)
				buttons |= XUSB_GAMEPAD_B;
			if (vpad_data.hold & VPAD_BUTTON_X)
				buttons |= XUSB_GAMEPAD_X;
			if (vpad_data.hold & VPAD_BUTTON_Y)
				buttons |= XUSB_GAMEPAD_Y;

			if (vpad_data.hold & VPAD_BUTTON_DOWN)
				buttons |= XUSB_GAMEPAD_DPAD_DOWN;
			if (vpad_data.hold & VPAD_BUTTON_UP)
				buttons |= XUSB_GAMEPAD_DPAD_UP;
			if (vpad_data.hold & VPAD_BUTTON_LEFT)
				buttons |= XUSB_GAMEPAD_DPAD_LEFT;
			if (vpad_data.hold & VPAD_BUTTON_RIGHT)
				buttons |= XUSB_GAMEPAD_DPAD_RIGHT;

			int size = 0;
			size += sprintf(send_buffer, "%.4X", buttons);
			udp_send(send_buffer, size);
		} else {
			if (vpad_data.trigger & VPAD_BUTTON_A) {
				if (udp_init("192.168.15.3", 4242)) {
					connected = 1;
				} else {
					OSScreenPutFontEx(SCREEN_DRC, -4, 3, "Connection Failed...");
					OSSleepTicks(10000000);
				}
			}
			OSScreenPutFontEx(SCREEN_DRC, -4, 1, "Press A to connect");
		}

		if (vpad_data.trigger & VPAD_BUTTON_HOME)
			break;

		// Flip buffers
		DCFlushRange(ScreenBuffer0, sBufferSizeTV);
		DCFlushRange(ScreenBuffer1, sBufferSizeDRC);
		OSScreenFlipBuffersEx(SCREEN_TV);
		OSScreenFlipBuffersEx(SCREEN_DRC);


	}

	udp_deinit();
	WHBDeinitializeSocketLibrary();
	OSScreenShutdown();
	MEMFreeByStateToFrmHeap(heap, FRAME_HEAP_TAG);
	WHBProcShutdown();

	return 0;
}

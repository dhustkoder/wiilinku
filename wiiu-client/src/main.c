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
#include <h264/decode.h>
#include "udp.h"

MEMHeapHandle heap;



#define FRAME_HEAP_TAG (0x000DECAF)

char send_buffer[42];


unsigned char test_img[1400];





int main(int argc, char **argv)
{
	WHBProcInit();
	WHBInitializeSocketLibrary();
	VPADInit();
	KPADInit();
	WPADEnableURCC(1);

	// Init screen and screen buffers
	heap = MEMGetBaseHeapHandle(MEM_BASE_HEAP_MEM1);
	MEMRecordStateForFrmHeap(heap, FRAME_HEAP_TAG);
	OSScreenInit();
	const uint32_t sBufferSizeTV = OSScreenGetBufferSizeEx(SCREEN_TV);
	const uint32_t sBufferSizeDRC = OSScreenGetBufferSizeEx(SCREEN_DRC);
	unsigned char *ScreenBuffer0 = MEMAllocFromFrmHeapEx(heap, sBufferSizeTV, 4);
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

	memset(test_img, 0x00, sizeof(test_img));

	for (;;) {
		VPADRead(VPAD_CHAN_0, &vpad_data, 1, &error);

		OSScreenClearBufferEx(SCREEN_TV, 0x000000FF);
		OSScreenClearBufferEx(SCREEN_DRC, 0x000000FF);
		OSScreenPutFontEx(SCREEN_DRC, -4, 0, "!Hello World!");

		if (connected) {
			OSScreenPutFontEx(SCREEN_DRC, -4, 1, "Connected.");
			OSScreenPutFontEx(SCREEN_DRC, -4, 2, send_buffer);

			const VPADVec2D ls = vpad_data.leftStick;
			const VPADVec2D rs = vpad_data.rightStick;
			int size = 0;
			size += sprintf(
				send_buffer,
				"%.5X %.3f %.3f %.3f %.3f",
				vpad_data.hold,
				ls.x, ls.y,
				rs.x, rs.y
			);

			udp_send((uint8_t*)send_buffer, sizeof send_buffer);
		} else {
			OSScreenPutFontEx(SCREEN_DRC, -4, 1, "Press A to connect");

			if (vpad_data.trigger & VPAD_BUTTON_A) {
				if (udp_init("192.168.15.7", 4242)) {
					OSScreenPutFontEx(SCREEN_DRC, -4, 3, "Connection Failed...");
					OSSleepTicks(10000000);
				} else {
					connected = 1;
				}
			}
			
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

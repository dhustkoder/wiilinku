#include <string.h>
#include <coreinit/screen.h>
#include <whb/libmanager.h>
#include <coreinit/memheap.h>
#include <coreinit/cache.h>
#include <coreinit/memfrmheap.h>
#include "video.h"

static MEMHeapHandle heap;
static uint32_t bufsz_tv;
static uint32_t bufsz_drc;
static unsigned char* buf_tv;
static unsigned char* buf_drc;


bool video_init(void)
{
	// Init screen and screen buffers
	heap = MEMGetBaseHeapHandle(MEM_BASE_HEAP_MEM1);
	MEMRecordStateForFrmHeap(heap, MEM_FRAME_HEAP_TAG);
	OSScreenInit();
	bufsz_tv = OSScreenGetBufferSizeEx(SCREEN_TV);
	bufsz_drc = OSScreenGetBufferSizeEx(SCREEN_DRC);
	buf_tv	= MEMAllocFromFrmHeapEx(heap, bufsz_tv, 4);
	buf_drc = MEMAllocFromFrmHeapEx(heap, bufsz_drc, 4);
	OSScreenSetBufferEx(SCREEN_TV, buf_tv);
	OSScreenSetBufferEx(SCREEN_DRC, buf_drc);
	OSScreenEnableEx(SCREEN_TV, 1);
	OSScreenEnableEx(SCREEN_DRC, 1);

	// Clear screens
	OSScreenClearBufferEx(SCREEN_TV, 0x000000FF);
	OSScreenClearBufferEx(SCREEN_DRC, 0x000000FF);

	return true;
}

void video_term(void)
{
	OSScreenShutdown();
	MEMFreeByStateToFrmHeap(heap, MEM_FRAME_HEAP_TAG);
}

void video_render_clear(void)
{
	OSScreenClearBufferEx(SCREEN_TV, 0x000000FF);
	OSScreenClearBufferEx(SCREEN_DRC, 0x000000FF);
}

void video_render_flip(void)
{
	DCFlushRange(buf_tv, bufsz_tv);
	DCFlushRange(buf_drc, bufsz_drc);
	OSScreenFlipBuffersEx(SCREEN_TV);
	OSScreenFlipBuffersEx(SCREEN_DRC);
}

void video_render_text(int x, int y, const char* buf)
{
	OSScreenPutFontEx(SCREEN_TV, x, y, buf);
	OSScreenPutFontEx(SCREEN_DRC, x, y, buf);
}

void video_render_text_aligned(int x, int y, char* buf)
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
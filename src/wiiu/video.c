#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <coreinit/screen.h>
#include <whb/libmanager.h>
#include <coreinit/memheap.h>
#include <coreinit/cache.h>
#include <coreinit/memfrmheap.h>
#include <coreinit/spinlock.h>
#include "video.h"


#define LOG_LINE_LENGTH (64)
#define LOG_NLINES      (4)
#define LOG_SCR_X       (0)
#define LOG_SCR_Y       (12)


static MEMHeapHandle heap;
static uint32_t bufsz_tv;
static uint32_t bufsz_drc;
static unsigned char* buf_tv;
static unsigned char* buf_drc;
static char fmt_buffer[512];
static char log_buffer[LOG_NLINES][LOG_LINE_LENGTH];
static OSSpinLock log_lock;




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


	OSInitSpinLock(&log_lock);

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
	video_render_text(LOG_SCR_X, LOG_SCR_Y, "-- log --");
	for (int i = 0; i < LOG_NLINES; ++i)
		video_render_text(LOG_SCR_X, LOG_SCR_Y + i + 1, log_buffer[i]);

	DCFlushRange(buf_tv, bufsz_tv);
	DCFlushRange(buf_drc, bufsz_drc);
	OSScreenFlipBuffersEx(SCREEN_TV);
	OSScreenFlipBuffersEx(SCREEN_DRC);
}

void video_render_text(int x, int y, const char* str)
{
	OSScreenPutFontEx(SCREEN_TV, x, y, str);
	OSScreenPutFontEx(SCREEN_DRC, x, y, str);
}

void video_render_text_aligned(int x, int y, const char* str)
{
	char* nwidx;

	while ((nwidx = strchr(str, '\n')) != NULL) {
		*nwidx = '\0';
		video_render_text(x, y++, str);
		*nwidx = '\n';
		str = nwidx + 1;
	}

	if (*str != '\0')
		video_render_text(x, y, str);
}


void video_render_text_fmt(int x, int y, const char* fmt, ...)
{
	FMT_STR_VARGS(fmt_buffer, sizeof(fmt_buffer), fmt, fmt);
	video_render_text(x, y, fmt_buffer);
}

void video_render_text_aligned_fmt(int x, int y, const char* fmt, ...)
{
	FMT_STR_VARGS(fmt_buffer, sizeof(fmt_buffer), fmt, fmt);
	video_render_text_aligned(x, y, fmt_buffer);
}

void video_log_printf(const char* fmt, ...)
{
	OSAcquireSpinLock(&log_lock);

	for (int i = 0; i < LOG_NLINES - 1; ++i) {
		strcpy(log_buffer[i], log_buffer[i + 1]);
	}

	FMT_STR_VARGS(log_buffer[LOG_NLINES - 1], LOG_LINE_LENGTH, fmt, fmt);

	OSReleaseSpinLock(&log_lock);
}


#ifndef WIIUPCX_ZUI_H_
#define WIIUPCX_ZUI_H_
#include <stdint.h>

struct zui_context {
	int w, h;
	void* fb;
} zctx;

struct zui_window {
	int w, h, x, y;
	void* pixels;
};

struct rgb24 {
	uint8_t b, g, r;
};

extern int zui_init(void* framebuffer, int w, int h);
extern int zui_window_init(struct zui_window* zw, int x, int y, int w, int h);
extern void zui_window_render(struct zui_window* win);
extern void zui_window_term(struct zui_window* win);
extern void zui_term(void);

#endif /* WIIUPCX_ZUI_H_ */

#ifdef ZUI_IMPLEMENTATION




int zui_init(void* framebuffer, int w, int h)
{
	zctx.fb = framebuffer;
	zctx.w = w;
	zctx.h = h;

	return 0;
}

int zui_window_init(struct zui_window* zw, int x, int y, int w, int h)
{
	zw->w = w;
	zw->h = h;
	zw->x = x;
	zw->y = y;

	zw->pixels = malloc(3 * w * h);
	memset(zw->pixels, 0, 3 * w * h);
	
	struct rgb24* p = zw->pixels;
	
	/* horizontal borders */
	for (int i = 0; i < w; ++i) {
		p[i].b = 0xFF;
		p[w + i].b = 0xFF;
		p[w * 2 + i].b = 0xFF;
		p[(w * (h - 1)) + i].b = 0xFF;
		p[(w * (h - 2)) + i].b = 0xFF;
		p[(w * (h - 3)) + i].b = 0xFF;
	}

	/* vertical borders */
	for (int i = 0; i < h; ++i) {
		p[i * w].b = 0xFF;
		p[i * w + 1].b = 0xFF;
		p[i * w + 2].b = 0xFF;
		p[(i * w) + (w - 1)].b = 0xFF;
		p[(i * w) + (w - 2)].b = 0xFF;
		p[(i * w) + (w - 3)].b = 0xFF;
	}


	return 0;
}

void zui_window_render(struct zui_window* zw)
{
	const int zwx = zw->x;
	const int zwy = zw->y;
	const int zww = zw->w;
	const int zwh = zw->h;

	const int fbw = zctx.w;
	const int fbh = zctx.h;

	struct rgb24* dest = zctx.fb;
	struct rgb24* src = zw->pixels;

	dest += (zwy * fbw) + zwx;

	for (int i = 0; i < zwh; ++i) {
		memcpy((void*)dest, (void*)src, zww * 3);
		src += zww;
		dest += fbw;
	}

}


void zui_window_term(struct zui_window* zw)
{
	free(zw->pixels);
}

void zui_term(void)
{

}


#endif


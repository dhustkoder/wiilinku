#ifndef WIIUPCX_HOST_GUI_H_
#define WIIUPCX_HOST_GUI_H_
#include <windows.h>
#include <stdint.h>

extern int gui_init(HINSTANCE hInstance, int nCmdShow);
extern BOOL gui_win_update(void);
extern void gui_term(void);
extern void gui_render(BYTE* data, int width, int height, int bpp);
extern void gui_log_write(const char* p);
#endif

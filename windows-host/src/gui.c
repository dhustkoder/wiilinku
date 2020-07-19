#include <windows.h>
#include <stdio.h>
#include "gui.h"
#include "log.h"


static HWND hwnd_mainwin;
static WNDCLASS wc;
static PAINTSTRUCT paintstruct;
static MSG msg_mainwin;
static BOOL wm_destroy_request = 0;
static DWORD win_width;
static DWORD win_height;

static HDC hdc_mainwin;
static BITMAPINFO bmi = {
	.bmiHeader = {
		.biSize = sizeof(BITMAPINFOHEADER),
		.biWidth = 0,
		.biHeight = 0,
		.biPlanes = 1,
		.biBitCount = 24,
		.biCompression = BI_RGB,
		.biSizeImage = 0,
		.biXPelsPerMeter = 0,
		.biYPelsPerMeter = 0,
		.biClrUsed = 0,
		.biClrImportant = 0
	}
};


static void window_size_update(void)
{
	RECT rect;
	GetClientRect(hwnd_mainwin, &rect);
	win_width = rect.right - rect.left;
	win_height = rect.bottom - rect.top;
}



static LRESULT window_proc_clbk(HWND hwnd,
                                UINT msg,
                                WPARAM wParam,
                                LPARAM lParam)
{
	switch (msg) {
		case WM_SIZE:
			window_size_update();
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			wm_destroy_request = 1;
			return WM_DESTROY;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

int gui_init(const HINSTANCE hInstance,
                       const int nCmdShow)
{
	memset(&wc, 0, sizeof(wc));
	wc.lpfnWndProc   = window_proc_clbk;
	wc.hInstance     = hInstance;
	wc.lpszClassName = "wiiupcx";

	RegisterClass(&wc);

	hwnd_mainwin = CreateWindowEx(
			0, wc.lpszClassName,
			"wiiupcx", WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
			NULL, NULL, wc.hInstance, NULL);
	if (!hwnd_mainwin) {
		printf("Failed to initialize WINDOW HWND");
		return 1;
	}

	ShowWindow(hwnd_mainwin, nCmdShow);
	UpdateWindow(hwnd_mainwin);
	window_size_update();

	hdc_mainwin = GetDC(hwnd_mainwin);

	return 0;
}

void gui_term(void)
{
	DestroyWindow(hwnd_mainwin);
}

BOOL gui_win_update(void)
{
	while (PeekMessageA(&msg_mainwin, hwnd_mainwin, 0, 0, PM_REMOVE))
		DispatchMessage(&msg_mainwin);
	return !wm_destroy_request;
}


extern void gui_render(BYTE* data, int width, int height, int bpp)
{
	BeginPaint(hwnd_mainwin, &paintstruct);

	bmi.bmiHeader.biWidth = width;
	bmi.bmiHeader.biHeight = -height;
	bmi.bmiHeader.biBitCount = (WORD) (bpp * 8);

	StretchDIBits(hdc_mainwin, 0, 0, win_width, win_height,
	              0, 0, width, height,
	              data, &bmi, DIB_RGB_COLORS, SRCCOPY);


	EndPaint(hwnd_mainwin, &paintstruct);
}



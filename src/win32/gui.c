#include <windows.h>
#include <stdio.h>
#include "connection.h"
#include "log.h"
#include "zui.h"
#include "gui.h"
#include "error.h"



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
		.biWidth = ZUI_WIDTH,
		.biHeight = -ZUI_HEIGHT,
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

static uint8_t framebuffer[ZUI_WIDTH * ZUI_HEIGHT * 3];

static zui_obj_id_t connection_status_text_id;
static zui_obj_id_t connection_local_ip_text_id;
static CRITICAL_SECTION zui_crit_sect;


static void window_size_update(void)
{
	RECT rect;
	GetClientRect(hwnd_mainwin, &rect);
	win_width = rect.right - rect.left;
	win_height = rect.bottom - rect.top;
}


static void flush_buffer(void)
{
	BeginPaint(hwnd_mainwin, &paintstruct);

	StretchDIBits(hdc_mainwin, 0, 0, win_width, win_height,
	              0, 0, ZUI_WIDTH, ZUI_HEIGHT,
	              framebuffer, &bmi, DIB_RGB_COLORS, SRCCOPY);


	EndPaint(hwnd_mainwin, &paintstruct);
}


static LRESULT window_proc_clbk(HWND hwnd,
                                UINT msg,
                                WPARAM wParam,
                                LPARAM lParam)
{
	switch (msg) {
		case WM_DESTROY:
			PostQuitMessage(0);
			wm_destroy_request = 1;
			return WM_DESTROY;
		case WM_SIZE:
			window_size_update();
		case WM_PAINT:
			flush_buffer();
			break;
	}	

	return DefWindowProc(hwnd, msg, wParam, lParam);
}



bool gui_init(void)
{ 
	memset(&wc, 0, sizeof(wc));
	wc.lpfnWndProc   = window_proc_clbk;
	wc.lpszClassName = TEXT("WiiLinkU " WIILINKU_VER_STR);

	RegisterClass(&wc);
	
	hwnd_mainwin = CreateWindowA(
		wc.lpszClassName,
		wc.lpszClassName,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		ZUI_WIDTH,
		ZUI_HEIGHT,
		NULL,
		NULL,
		wc.hInstance,
		NULL
	);

	if (!hwnd_mainwin) {
		set_last_error("CreateWindowEx failed");
		return false;
	}

	ShowWindow(hwnd_mainwin, 1);
	UpdateWindow(hwnd_mainwin);
	window_size_update();

	hdc_mainwin = GetDC(hwnd_mainwin);

	zui_init();

	connection_local_ip_text_id = zui_text_create(64, 1, (struct vec2i){ 50, 16 });
	connection_status_text_id = zui_text_create(64, 1, (struct vec2i){ 50, 32 });

	InitializeCriticalSectionAndSpinCount(&zui_crit_sect, ~(DWORD)0);

	gui_set_connection_status(false, NULL);
	gui_set_connection_local_ip(NULL);

	return true;
}

void gui_term(void)
{
	DeleteCriticalSection(&zui_crit_sect);
	zui_term();
	DestroyWindow(hwnd_mainwin);
}

void gui_set_connection_status(bool connected, const char* clientaddr)
{
	EnterCriticalSection(&zui_crit_sect);
	if (!connected) {
		zui_text_set(
			connection_status_text_id,
			"Connection status: Waiting For Client"
		);
	} else {
		char buf[64];
		sprintf(buf, "Connection Status: Connected to %s", clientaddr);
		zui_text_set(connection_status_text_id, buf);
	}
	LeaveCriticalSection(&zui_crit_sect);
}

void gui_set_connection_local_ip(const char* ip)
{
	EnterCriticalSection(&zui_crit_sect);
	if (ip == NULL) {
		zui_text_set(
			connection_local_ip_text_id,
			"Getting local IP..."
		);
	} else {
		char buf[64];
		sprintf(buf, "Your PC IP: %s", ip);
		zui_text_set(
			connection_local_ip_text_id,
			buf
		);
	}
	LeaveCriticalSection(&zui_crit_sect);
}

gui_event_t gui_update(void)
{
	if (wm_destroy_request)
		return GUI_EVENT_WM_DESTROY;

	while (PeekMessageA(&msg_mainwin, hwnd_mainwin, 0, 0, PM_REMOVE))
		DispatchMessage(&msg_mainwin);

	if (zui_update()) {
		EnterCriticalSection(&zui_crit_sect);
		zui_render((void*)framebuffer);
		LeaveCriticalSection(&zui_crit_sect);
		flush_buffer();
	}

	return 0;
}



#include <windows.h>
#include <stdio.h>
#include "connection.h"
#include "log.h"
#include "zui.h"
#include "gui.h"
#include "error.h"


struct gui_text {
	zui_obj_id_t id;
	struct vec2i origin;
	const char* str;
};

struct gui_text gui_header_txts[] = {
	{
		.origin = { .x = ZUI_WIDTH / 2, .y = 22 },
		.str = "WiiLinkU " WIILINKU_VER_STR
	},
	{
		.origin = { .x = ZUI_WIDTH / 2, .y = 44 },
		.str = "Loading local IP..."
	},
	{
		.origin = { .x = ZUI_WIDTH / 2, .y = 64 },
		.str = "Connection Status: Not Connected"
	}
};

struct gui_text gui_joy_txts[] = {
	{
		.origin = { .x = 96, .y = 128 },
		.str = "Controllers Connected: 0"
	},
	{
		.origin = { .x = 96, .y = 138 },
		.str = "WiiU Gamepad"
	},
	{
		.origin = { .x = 96, .y = 148 },
		.str = "Wiimote 1"
	},
	{
		.origin = { .x = 96, .y = 158 },
		.str = "Wiimote 2"
	},
	{
		.origin = { .x = 96, .y = 168 },
		.str = "Wiimote 3"
	},
	{
		.origin = { .x = 96, .y = 178 },
		.str = "Wiimote 4"
	},
};



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
	wc.lpfnWndProc   = (void*)window_proc_clbk;
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

	for (int i = 0; i < (sizeof(gui_header_txts)/sizeof(gui_header_txts[0])); ++i) {
		gui_header_txts[i].id = zui_text_create(gui_header_txts[i].origin);
		zui_text_set(gui_header_txts[i].id, gui_header_txts[i].str);
		zui_text_draw(gui_header_txts[i].id);
	}

	for (int i = 0; i < (sizeof(gui_joy_txts)/sizeof(gui_joy_txts[0])); ++i) {
		gui_joy_txts[i].id = zui_text_create(gui_joy_txts[i].origin);
		zui_text_set(gui_joy_txts[i].id, gui_joy_txts[i].str);
	}


	InitializeCriticalSectionAndSpinCount(&zui_crit_sect, ~(DWORD)0);

	return true;
}

void gui_term(void)
{
	DeleteCriticalSection(&zui_crit_sect);
	zui_term();
	DestroyWindow(hwnd_mainwin);
}

void gui_set_local_ip_string(const char* ip)
{
	char buf[64];
	if (ip != NULL)
		sprintf(buf, "Your IP: %s", ip);
	else
		sprintf(buf, "Couldn't load your local IP. Try ipconfig");

	EnterCriticalSection(&zui_crit_sect);
	zui_text_set(gui_header_txts[1].id, buf);
	zui_text_draw(gui_header_txts[1].id);
	LeaveCriticalSection(&zui_crit_sect);
}

void gui_set_client_ip_string(const char* client_ip)
{
	char buf[64];

	if (client_ip != NULL) 
		sprintf(buf, "Connection Status: Connected to %s", client_ip);
	else
		strcpy(buf, gui_header_txts[2].str);

	EnterCriticalSection(&zui_crit_sect);
	zui_text_set(gui_header_txts[2].id, buf);
	zui_text_draw(gui_header_txts[2].id);
	LeaveCriticalSection(&zui_crit_sect);
}

void gui_set_connected_controllers(input_packet_flags_t flags)
{
	static const input_packet_flags_t joyflags[] = {
		INPUT_PACKET_FLAG_GAMEPAD,
		INPUT_PACKET_FLAG_WIIMOTE_0,
		INPUT_PACKET_FLAG_WIIMOTE_1,
		INPUT_PACKET_FLAG_WIIMOTE_2,
		INPUT_PACKET_FLAG_WIIMOTE_3
	};

	static input_packet_flags_t last_flags = 0;

	char buf[32];

	if (last_flags == flags)
		return;

	last_flags = flags;

	EnterCriticalSection(&zui_crit_sect);

	int nconn = 0;
	
	for (int i = 0; i < sizeof(joyflags)/sizeof(joyflags[0]); ++i) {
		if (flags&joyflags[i]) {
			++nconn;
			zui_text_draw(gui_joy_txts[i + 1].id);
		} else {
			zui_text_erase(gui_joy_txts[i + 1].id);
		}
	}
	
	if (nconn > 0) {
		sprintf(buf, "Controllers Connected: %d", nconn);
		zui_text_set(gui_joy_txts[0].id, buf);
		zui_text_draw(gui_joy_txts[0].id);
	} else {
		zui_text_erase(gui_joy_txts[0].id);
	}
	
	LeaveCriticalSection(&zui_crit_sect);

	
}


gui_event_t gui_update(void)
{
	if (wm_destroy_request)
		return GUI_EVENT_WM_DESTROY;

	while (PeekMessageA(&msg_mainwin, hwnd_mainwin, 0, 0, PM_REMOVE))
		DispatchMessage(&msg_mainwin);

	EnterCriticalSection(&zui_crit_sect);
	if (zui_update()) {
		zui_render((void*)framebuffer);
		flush_buffer();
	}
	LeaveCriticalSection(&zui_crit_sect);

	return 0;
}



#include <stdint.h>
#include <limits.h>
#include <stdio.h>
#include "gui.h"
#include "log.h"
#include "x360emu.h"

#define NETN_IMPLEMENTATION
#include "netn.h"


static BYTE vout_buffer[1366 * 768 * 3];
static BYTE vin_buffer[1366 * 768 * 3];


static void get_current_frame(BYTE* dest, int* x, int* y, int *bpp)
{
	
	int ScreenX = 0;
	int ScreenY = 0;

    HDC hScreen = GetDC(NULL);
    ScreenX = 800; //GetDeviceCaps(hScreen, HORZRES);
    ScreenY = 600; //GetDeviceCaps(hScreen, VERTRES);

    HDC hdcMem = CreateCompatibleDC(hScreen);
    HBITMAP hBitmap = CreateCompatibleBitmap(hScreen, ScreenX, ScreenY);
    HGDIOBJ hOld = SelectObject(hdcMem, hBitmap);
    BitBlt(hdcMem, 0, 0, ScreenX, ScreenY, hScreen, 0, 0, SRCCOPY);
    SelectObject(hdcMem, hOld);

    BITMAPINFOHEADER bmi = {0};
    bmi.biSize = sizeof(BITMAPINFOHEADER);
    bmi.biPlanes = 1;
    bmi.biBitCount = 24;
    bmi.biWidth = ScreenX;
    bmi.biHeight = -ScreenY;
    bmi.biCompression = BI_RGB;
    bmi.biSizeImage = 0;// 3 * ScreenX * ScreenY;

    GetDIBits(hdcMem, hBitmap, 0, ScreenY, dest, (BITMAPINFO*)&bmi, DIB_RGB_COLORS);
	*x = ScreenX;
	*y = ScreenY;
	*bpp = 3;

    ReleaseDC(GetDesktopWindow(),hScreen);
    DeleteDC(hdcMem);
    DeleteObject(hBitmap);
}



static void run_video_sender_thread(void)
{

	int x, y , bpp;
	for (;;)  {
		if (!gui_win_update())
			break;
		

		get_current_frame(vout_buffer, &x, &y, &bpp);
		log_info("got frame\n");

		size_t frame_size = x * y * bpp;
		size_t sender_idx = 0, recver_idx = 0;
		unsigned char* voutp = vout_buffer;
		unsigned char* vinp = vin_buffer;

		while (sender_idx < frame_size || recver_idx < frame_size) {
			netn_send(voutp, 1400, NETN_CONNECTION_VOUT);
			voutp += 1400;
			sender_idx += 1400;

			netn_recv(vinp, 1400, NETN_CONNECTION_VOUT);
			vinp += 1400;
			recver_idx += 1400;
		}


		log_info("RECEIVED!\n");
		
		gui_render(vin_buffer, x, y, bpp);
		log_info("framed\n");
	}


}


static int init_platform(HINSTANCE hins, int ncmd)
{
	if (log_init())
		return 1;

	//if (gui_init(hins, ncmd))
	//	return 1;
	
	if (x360emu_init())
		return 1;

	if (netn_init())
		return 1;

	return 0;
}

static void terminate_platform(void)
{
	netn_term();
	x360emu_term();
	//gui_term();
	log_term();
}

int CALLBACK WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR lpCmdLine,
                     const int nCmdShow)
{
	((void)hPrevInstance);
	((void)lpCmdLine);

	if (init_platform(hInstance, nCmdShow))
		return 1;
		
	
	struct netn_joy_packet jpkt;

	for (;;) {
		if (netn_joy_update(&jpkt)) {
			log_info("ERROR RECV INPUT\n");
		}

		log_info(
			"GAMEPAD: %.8X %.4X %.4X %.4X %.4X\n"
			"WIIMOTE: %.8X\n",
			jpkt.gamepad.btns, jpkt.gamepad.lsx, jpkt.gamepad.lsy, jpkt.gamepad.rsx, jpkt.gamepad.rsy,
			jpkt.wiimote.btns
		);

		x360emu_update(&jpkt);
	}


	terminate_platform();

	return 0;
}

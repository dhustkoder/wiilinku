#include <stdint.h>
#include <limits.h>
#include <stdio.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define NETN_IMPLEMENTATION
#include "netn.h"


#include "gui.h"
#include "log.h"
#include "video_encoder.h"
#include "x360emu.h"



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


static void run_input_recv_thread(void)
{
	unsigned char recv_buf[42];
	uint32_t wiiu_btns;
	struct vec2 ls;
	struct vec2 rs;
	int bytes_read;

	for (;;) {
		ls = (struct vec2) { 0, 0 };
		rs = (struct vec2) { 0, 0 };
		wiiu_btns = 0;

		if ((bytes_read = netn_recv_packet(recv_buf, sizeof recv_buf, NETN_CONNECTION_JIN)) != -1) {
			_snscanf(
				(char*)&recv_buf[0],
				sizeof recv_buf,
				"%X %f %f %f %f",
				&wiiu_btns,
				&ls.x, &ls.y,
				&rs.x, &rs.y
			);

			log_info(
				"RECV: %.5X %.3f %.3f %.3f %.3f | TOTAL BUFFER SIZE = %d\n",
				wiiu_btns, ls.x, ls.y, rs.x, rs.y, bytes_read
			);
		}

		x360emu_update(wiiu_btns, ls, rs);
	}
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
			netn_send_packet(voutp, 1400, NETN_CONNECTION_VOUT);
			voutp += 1400;
			sender_idx += 1400;

			netn_recv_packet(vinp, 1400, NETN_CONNECTION_VOUT);
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
		
	
	run_input_recv_thread();

	// run_video_sender_thread();

	terminate_platform();

	return 0;
}

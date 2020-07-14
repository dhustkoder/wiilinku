#include <winsock2.h>
#include <ViGEm/Client.h>
#include <stdio.h>
#include <stdint.h>
#include <limits.h>
#define ZED_NET_IMPLEMENTATION

#include "zed_net.h"

enum wiiu_button {
	WIIU_BUTTON_A     = 0x8000,
	WIIU_BUTTON_B     = 0x4000,
	WIIU_BUTTON_X     = 0x2000,
	WIIU_BUTTON_Y     = 0x1000,
	WIIU_BUTTON_LEFT  = 0x0800,
	WIIU_BUTTON_RIGHT = 0x0400,
	WIIU_BUTTON_UP    = 0x0200,
	WIIU_BUTTON_DOWN  = 0x0100,
	WIIU_BUTTON_ZL    = 0x0080,
	WIIU_BUTTON_ZR    = 0x0040,
	WIIU_BUTTON_L     = 0x0020,
	WIIU_BUTTON_R     = 0x0010,
	WIIU_BUTTON_PLUS  = 0x0008,
	WIIU_BUTTON_MINUS = 0x0004,
	WIIU_BUTTON_STICK_R = 0x00020000,
	WIIU_BUTTON_STICK_L = 0x00040000
};

struct vec2 {
	float x,y;
} ls, rs;


char recv_buffer[256];


VOID CALLBACK notification(
	PVIGEM_CLIENT Client,
	PVIGEM_TARGET Target,
	UCHAR LargeMotor,
	UCHAR SmallMotor,
	UCHAR LedNumber,
	LPVOID UserData
)
{
	printf("NOTIFICATION CALLED\n");
}


int main(int argc, char **argv) 
{

	PVIGEM_CLIENT client = vigem_alloc();
	VIGEM_ERROR ret = vigem_connect(client);
	PVIGEM_TARGET x360 = vigem_target_x360_alloc();
	ret = vigem_target_add(client, x360);
	ret = vigem_target_x360_register_notification(client, x360, &notification, NULL);

	XUSB_REPORT report;
	XUSB_REPORT_INIT(&report);
	zed_net_init();

	const unsigned short port = 4242;
	zed_net_socket_t socket;
	if (zed_net_udp_socket_open(&socket, port, 0)) {
		printf("Error: %s\n", zed_net_get_error());
		return -1;
	}

	printf("waiting data on port: %d!\n", port);

	for (;;) {
		zed_net_address_t sender;
		int bytes_read = zed_net_udp_socket_receive(
			&socket,
			&sender,
			&recv_buffer,
			sizeof(recv_buffer)
		);

		if (bytes_read) {
			uint32_t wiiu_btns;

			sscanf(
				recv_buffer,
				"%X %f %f %f %f",
				&wiiu_btns,
				&ls.x, &ls.y,
				&rs.x, &rs.y
			);

			report.wButtons = 0x00;

			if (wiiu_btns & WIIU_BUTTON_A)
				report.wButtons |= XUSB_GAMEPAD_B;
			if (wiiu_btns & WIIU_BUTTON_B)
				report.wButtons |= XUSB_GAMEPAD_A;
			if (wiiu_btns & WIIU_BUTTON_X)
				report.wButtons |= XUSB_GAMEPAD_Y;
			if (wiiu_btns & WIIU_BUTTON_Y)
				report.wButtons |= XUSB_GAMEPAD_X;

			if (wiiu_btns & WIIU_BUTTON_DOWN)
				report.wButtons |= XUSB_GAMEPAD_DPAD_DOWN;
			if (wiiu_btns & WIIU_BUTTON_UP)
				report.wButtons |= XUSB_GAMEPAD_DPAD_UP;
			if (wiiu_btns & WIIU_BUTTON_LEFT)
				report.wButtons |= XUSB_GAMEPAD_DPAD_LEFT;
			if (wiiu_btns & WIIU_BUTTON_RIGHT)
				report.wButtons |= XUSB_GAMEPAD_DPAD_RIGHT;

			if (wiiu_btns & WIIU_BUTTON_PLUS)
				report.wButtons |= XUSB_GAMEPAD_START;
			if (wiiu_btns & WIIU_BUTTON_MINUS)
				report.wButtons |= XUSB_GAMEPAD_BACK;
			if (wiiu_btns & WIIU_BUTTON_L)
				report.wButtons |= XUSB_GAMEPAD_LEFT_SHOULDER;
			if (wiiu_btns & WIIU_BUTTON_R)
				report.wButtons |= XUSB_GAMEPAD_RIGHT_SHOULDER;

			if (wiiu_btns & WIIU_BUTTON_STICK_R)
				report.wButtons |= XUSB_GAMEPAD_RIGHT_THUMB;
			if (wiiu_btns & WIIU_BUTTON_STICK_L)
				report.wButtons |= XUSB_GAMEPAD_LEFT_THUMB;

			report.bLeftTrigger = (wiiu_btns&WIIU_BUTTON_ZL) ? 0xFF : 0x00;
			report.bRightTrigger = (wiiu_btns&WIIU_BUTTON_ZR) ? 0xFF : 0x00;
			report.sThumbLX = ls.x * INT16_MAX;
			report.sThumbLY = ls.y * INT16_MAX;
			report.sThumbRX = rs.x * INT16_MAX;
			report.sThumbRY = rs.y * INT16_MAX;

			printf(
				"RECV: %.5X %.3f %.3f %.3f %.3f | TOTAL BUFFER SIZE = %d\n",
				wiiu_btns, ls.x, ls.y, rs.x, rs.y, bytes_read
			);

		}


		ret = vigem_target_x360_update(client, x360, report);

	}

	vigem_target_x360_unregister_notification(x360);
	vigem_target_remove(client, x360);
	vigem_target_free(x360);
	vigem_free(client);

	zed_net_socket_close(&socket);
	zed_net_shutdown();
	return 0;
}

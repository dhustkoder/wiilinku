#include <winsock2.h>
#include <windows.h>
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

struct vigem_xbox_controller {
	PVIGEM_CLIENT client;
	PVIGEM_TARGET target;
	XUSB_REPORT report;
} x360_pad;

#define IN_CONN_RECV_BUF_SIZE (42)
struct input_connection {
	zed_net_socket_t socket;
	zed_net_address_t sender;
	char recv_buf[IN_CONN_RECV_BUF_SIZE];
} in_conn;


VOID CALLBACK xbox_controller_notification_handler(
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


static int initialize_vigem_xbox_controller(void)
{
	x360_pad.client = vigem_alloc();
	VIGEM_ERROR err = vigem_connect(x360_pad.client);

	if (err != VIGEM_ERROR_NONE) {
		printf("vigem_connect error = %X\n", err);
		return 1;
	}

	x360_pad.target = vigem_target_x360_alloc();
	err = vigem_target_add(x360_pad.client, x360_pad.target);

	if (err != VIGEM_ERROR_NONE) {
		printf("vigem_target_add error = %X\n", err);
		return 1;
	}

	err = vigem_target_x360_register_notification(
		x360_pad.client,
		x360_pad.target,
		&xbox_controller_notification_handler,
		NULL
	);

	if (err != VIGEM_ERROR_NONE) {
		printf("vigem_target_x360_register_notification error = %X\n", err);
		return 1;
	}

	XUSB_REPORT_INIT(&x360_pad.report);

	return 0;
}

static void terminate_vigem_xbox_controller(void)
{
	vigem_target_x360_unregister_notification(x360_pad.target);
	vigem_target_remove(x360_pad.client, x360_pad.target);
	vigem_target_free(x360_pad.target);
	vigem_free(x360_pad.client);
}

static void update_vigem_xbox_controller_state(
	const uint32_t wiiu_btns,
	struct vec2 ls,
	struct vec2 rs
)
{
	x360_pad.report.wButtons = 0x00;

	if (wiiu_btns & WIIU_BUTTON_A)
		x360_pad.report.wButtons |= XUSB_GAMEPAD_B;
	if (wiiu_btns & WIIU_BUTTON_B)
		x360_pad.report.wButtons |= XUSB_GAMEPAD_A;
	if (wiiu_btns & WIIU_BUTTON_X)
		x360_pad.report.wButtons |= XUSB_GAMEPAD_Y;
	if (wiiu_btns & WIIU_BUTTON_Y)
		x360_pad.report.wButtons |= XUSB_GAMEPAD_X;

	if (wiiu_btns & WIIU_BUTTON_DOWN)
		x360_pad.report.wButtons |= XUSB_GAMEPAD_DPAD_DOWN;
	if (wiiu_btns & WIIU_BUTTON_UP)
		x360_pad.report.wButtons |= XUSB_GAMEPAD_DPAD_UP;
	if (wiiu_btns & WIIU_BUTTON_LEFT)
		x360_pad.report.wButtons |= XUSB_GAMEPAD_DPAD_LEFT;
	if (wiiu_btns & WIIU_BUTTON_RIGHT)
		x360_pad.report.wButtons |= XUSB_GAMEPAD_DPAD_RIGHT;

	if (wiiu_btns & WIIU_BUTTON_PLUS)
		x360_pad.report.wButtons |= XUSB_GAMEPAD_START;
	if (wiiu_btns & WIIU_BUTTON_MINUS)
		x360_pad.report.wButtons |= XUSB_GAMEPAD_BACK;
	if (wiiu_btns & WIIU_BUTTON_L)
		x360_pad.report.wButtons |= XUSB_GAMEPAD_LEFT_SHOULDER;
	if (wiiu_btns & WIIU_BUTTON_R)
		x360_pad.report.wButtons |= XUSB_GAMEPAD_RIGHT_SHOULDER;

	if (wiiu_btns & WIIU_BUTTON_STICK_R)
		x360_pad.report.wButtons |= XUSB_GAMEPAD_RIGHT_THUMB;
	if (wiiu_btns & WIIU_BUTTON_STICK_L)
		x360_pad.report.wButtons |= XUSB_GAMEPAD_LEFT_THUMB;

	x360_pad.report.bLeftTrigger = (wiiu_btns&WIIU_BUTTON_ZL) ? 0xFF : 0x00;
	x360_pad.report.bRightTrigger = (wiiu_btns&WIIU_BUTTON_ZR) ? 0xFF : 0x00;
	x360_pad.report.sThumbLX = ls.x * INT16_MAX;
	x360_pad.report.sThumbLY = ls.y * INT16_MAX;
	x360_pad.report.sThumbRX = rs.x * INT16_MAX;
	x360_pad.report.sThumbRY = rs.y * INT16_MAX;

	VIGEM_ERROR ret = vigem_target_x360_update(
		x360_pad.client,
		x360_pad.target,
		x360_pad.report
	);

	if (ret != VIGEM_ERROR_NONE) {
		printf("Error on vigem target update: %X\n", ret);
	}
}


static int initialize_zed_net(void)
{
	if (zed_net_init()) {
		printf("error initializing zed net\n");
		return 1;
	}

	if (zed_net_udp_socket_open(&in_conn.socket, 4242, 0)) {
		printf("Error: %s\n", zed_net_get_error());
		return 1;
	}
	printf("waiting data on port: %d!\n", 4242);
	return 0;
}

static void terminate_zed_net(void)
{
	zed_net_socket_close(&in_conn.socket);
	zed_net_shutdown();
}

static void run_input_recv_thread(void)
{
	for (;;) {
		int bytes_read = zed_net_udp_socket_receive(
			&in_conn.socket,
			&in_conn.sender,
			&in_conn.recv_buf[0],
			IN_CONN_RECV_BUF_SIZE
		);

		uint32_t wiiu_btns = 0x00;
		struct vec2 ls = { 0, 0 };
		struct vec2 rs = { 0, 0 };

		if (bytes_read) {
			sscanf(
				in_conn.recv_buf,
				"%X %f %f %f %f",
				&wiiu_btns,
				&ls.x, &ls.y,
				&rs.x, &rs.y
			);

			printf(
				"RECV: %.5X %.3f %.3f %.3f %.3f | TOTAL BUFFER SIZE = %d\n",
				wiiu_btns, ls.x, ls.y, rs.x, rs.y, bytes_read
			);
		}

		update_vigem_xbox_controller_state(wiiu_btns, ls, rs);
	}
}


int main(int argc, char **argv) 
{
	if (initialize_zed_net())
		return EXIT_FAILURE;

	if (initialize_vigem_xbox_controller())
		return EXIT_FAILURE;

	
	run_input_recv_thread();

	terminate_vigem_xbox_controller();
	terminate_zed_net();

	return 0;
}

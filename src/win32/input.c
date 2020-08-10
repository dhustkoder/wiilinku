#include <windows.h>
#include <ViGEm/Client.h>
#include "input.h"
#include "connection.h"
#include "log.h"
#include "error.h"


static struct input_packet last_input;


static struct vigem_pad {
	PVIGEM_CLIENT client;
	PVIGEM_TARGET target;
	XUSB_REPORT report;
} vigem_pad;


static VOID CALLBACK vigem_pad_notification(
	PVIGEM_CLIENT client,
	PVIGEM_TARGET target,
	UCHAR large_motor,
	UCHAR small_motor,
	UCHAR led_number,
	LPVOID udata
)
{
	__pragma(warning(suppress:4100)) client;
	__pragma(warning(suppress:4100)) target;
	__pragma(warning(suppress:4100)) large_motor;
	__pragma(warning(suppress:4100)) small_motor;
	__pragma(warning(suppress:4100)) led_number;
	__pragma(warning(suppress:4100)) udata;

	struct input_feedback_packet feedback = {
		.placeholder = large_motor|small_motor
	};

	connection_send_input_feedback_packet(&feedback);

	log_debug(
		"X360 NOTIFICATION CALLED: \n"
		"%.2X large_motor\n"
		"%.2X small_motor\n",
		(unsigned)large_motor,
		(unsigned)small_motor
	);
}



bool input_init(void)
{
	vigem_pad.client = vigem_alloc();
	VIGEM_ERROR err = vigem_connect(vigem_pad.client);

	if (err != VIGEM_ERROR_NONE) {
		log_error("vigem_connect error = %X\n", err);
		goto Lretfail;
	}

	vigem_pad.target = vigem_target_x360_alloc();
	err = vigem_target_add(vigem_pad.client, vigem_pad.target);

	if (err != VIGEM_ERROR_NONE) {
		log_error("vigem_target_add error = %X\n", err);
		goto Lretfail;
	}

	err = vigem_target_x360_register_notification(
		vigem_pad.client,
		vigem_pad.target,
		&vigem_pad_notification,
		NULL
	);

	if (err != VIGEM_ERROR_NONE) {
		log_error("vigem_target_x360_register_notification error = %X\n", err);
		goto Lretfail;
	}

	XUSB_REPORT_INIT(&vigem_pad.report);


	return true;

Lretfail:
	set_last_error(
		"failed to initialize Input system."
		"Make sure you've installed ViGEmBus driver."
	);

	return false;
}


void input_term(void)
{
	vigem_target_x360_unregister_notification(vigem_pad.target);
	vigem_target_remove(vigem_pad.client, vigem_pad.target);
	vigem_target_free(vigem_pad.target);
	vigem_free(vigem_pad.client);
}



void input_update(const struct input_packet* input)
{
	if (memcmp(&last_input, input, sizeof last_input) == 0)
		return;

	memcpy(&last_input, input, sizeof last_input);

	log_debug(
		"GAMEPAD BTNS: %.8X\n"
		"WIIMOTE[0] BTNS: %.8X\n",
		last_input.gamepad.btns,
		last_input.wiimotes[0].btns
	);

	const uint32_t wiiu_btns = last_input.gamepad.btns;
	const uint32_t wiimote_btns = last_input.wiimotes[0].btns;

	vigem_pad.report.wButtons = 0x00;

	if (wiiu_btns & WIIU_GAMEPAD_BTN_A || wiimote_btns & WIIMOTE_BTN_2)
		vigem_pad.report.wButtons |= XUSB_GAMEPAD_B;
	if (wiiu_btns & WIIU_GAMEPAD_BTN_B || wiimote_btns & WIIMOTE_BTN_1)
		vigem_pad.report.wButtons |= XUSB_GAMEPAD_A;
	if (wiiu_btns & WIIU_GAMEPAD_BTN_X || wiimote_btns & WIIMOTE_BTN_A)
		vigem_pad.report.wButtons |= XUSB_GAMEPAD_Y;
	if (wiiu_btns & WIIU_GAMEPAD_BTN_Y || wiimote_btns & WIIMOTE_BTN_B)
		vigem_pad.report.wButtons |= XUSB_GAMEPAD_X;

	if (wiiu_btns & WIIU_GAMEPAD_BTN_DOWN || wiimote_btns & WIIMOTE_BTN_LEFT)
		vigem_pad.report.wButtons |= XUSB_GAMEPAD_DPAD_DOWN;
	if (wiiu_btns & WIIU_GAMEPAD_BTN_UP || wiimote_btns & WIIMOTE_BTN_RIGHT)
		vigem_pad.report.wButtons |= XUSB_GAMEPAD_DPAD_UP;
	if (wiiu_btns & WIIU_GAMEPAD_BTN_LEFT || wiimote_btns & WIIMOTE_BTN_UP)
		vigem_pad.report.wButtons |= XUSB_GAMEPAD_DPAD_LEFT;
	if (wiiu_btns & WIIU_GAMEPAD_BTN_RIGHT || wiimote_btns & WIIMOTE_BTN_DOWN)
		vigem_pad.report.wButtons |= XUSB_GAMEPAD_DPAD_RIGHT;

	if (wiiu_btns & WIIU_GAMEPAD_BTN_PLUS || wiimote_btns & WIIMOTE_BTN_PLUS)
		vigem_pad.report.wButtons |= XUSB_GAMEPAD_START;
	if (wiiu_btns & WIIU_GAMEPAD_BTN_MINUS || wiimote_btns & WIIMOTE_BTN_MINUS)
		vigem_pad.report.wButtons |= XUSB_GAMEPAD_BACK;
	if (wiiu_btns & WIIU_GAMEPAD_BTN_L)
		vigem_pad.report.wButtons |= XUSB_GAMEPAD_LEFT_SHOULDER;
	if (wiiu_btns & WIIU_GAMEPAD_BTN_R)
		vigem_pad.report.wButtons |= XUSB_GAMEPAD_RIGHT_SHOULDER;

	if (wiiu_btns & WIIU_GAMEPAD_BTN_STICK_R)
		vigem_pad.report.wButtons |= XUSB_GAMEPAD_RIGHT_THUMB;
	if (wiiu_btns & WIIU_GAMEPAD_BTN_STICK_L)
		vigem_pad.report.wButtons |= XUSB_GAMEPAD_LEFT_THUMB;

	vigem_pad.report.bLeftTrigger = (wiiu_btns&WIIU_GAMEPAD_BTN_ZL) ? 0xFF : 0x00;
	vigem_pad.report.bRightTrigger = (wiiu_btns&WIIU_GAMEPAD_BTN_ZR) ? 0xFF : 0x00;
	vigem_pad.report.sThumbLX = last_input.gamepad.lsx;
	vigem_pad.report.sThumbLY = last_input.gamepad.lsy;
	vigem_pad.report.sThumbRX = last_input.gamepad.rsx;
	vigem_pad.report.sThumbRY = last_input.gamepad.rsy;

	VIGEM_ERROR ret = vigem_target_x360_update(
		vigem_pad.client,
		vigem_pad.target,
		vigem_pad.report
	);

	if (ret != VIGEM_ERROR_NONE) {
		log_debug("Error on vigem target update: %X\n", ret);
	}
}
#include <windows.h>
#include <ViGEm/Client.h>
#include "x360emu.h"
#include "log.h"

static struct vigem_x360_pad {
	PVIGEM_CLIENT client;
	PVIGEM_TARGET target;
	XUSB_REPORT report;
} x360_pad;

static VOID CALLBACK x360_notification(
	PVIGEM_CLIENT client,
	PVIGEM_TARGET target,
	UCHAR large_motor,
	UCHAR small_motor,
	UCHAR led_number,
	LPVOID udata
)
{
	((void)client);
	((void)target);
	((void)large_motor);
	((void)small_motor);
	((void)led_number);
	((void)udata);
	log_info("X360 NOTIFICATION CALLED");
}



bool x360emu_init(void)
{
	x360_pad.client = vigem_alloc();
	VIGEM_ERROR err = vigem_connect(x360_pad.client);

	if (err != VIGEM_ERROR_NONE) {
		log_info("vigem_connect error = %X\n", err);
		return false;
	}

	x360_pad.target = vigem_target_x360_alloc();
	err = vigem_target_add(x360_pad.client, x360_pad.target);

	if (err != VIGEM_ERROR_NONE) {
		log_info("vigem_target_add error = %X\n", err);
		return false;
	}

	err = vigem_target_x360_register_notification(
		x360_pad.client,
		x360_pad.target,
		&x360_notification,
		NULL
	);

	if (err != VIGEM_ERROR_NONE) {
		log_info("vigem_target_x360_register_notification error = %X\n", err);
		return false;
	}

	XUSB_REPORT_INIT(&x360_pad.report);

	return true;
}


void x360emu_term(void)
{
	vigem_target_x360_unregister_notification(x360_pad.target);
	vigem_target_remove(x360_pad.client, x360_pad.target);
	vigem_target_free(x360_pad.target);
	vigem_free(x360_pad.client);
}

void x360emu_update(struct input_packet* pack)
{
	const uint32_t wiiu_btns = pack->gamepad.btns;
	const uint32_t wiimote_btns = pack->wiimotes[0].btns;

	x360_pad.report.wButtons = 0x00;

	if (wiiu_btns & WIIU_GAMEPAD_BTN_A || wiimote_btns & WIIMOTE_BTN_2)
		x360_pad.report.wButtons |= XUSB_GAMEPAD_B;
	if (wiiu_btns & WIIU_GAMEPAD_BTN_B || wiimote_btns & WIIMOTE_BTN_1)
		x360_pad.report.wButtons |= XUSB_GAMEPAD_A;
	if (wiiu_btns & WIIU_GAMEPAD_BTN_X || wiimote_btns & WIIMOTE_BTN_A)
		x360_pad.report.wButtons |= XUSB_GAMEPAD_Y;
	if (wiiu_btns & WIIU_GAMEPAD_BTN_Y || wiimote_btns & WIIMOTE_BTN_B)
		x360_pad.report.wButtons |= XUSB_GAMEPAD_X;

	if (wiiu_btns & WIIU_GAMEPAD_BTN_DOWN || wiimote_btns & WIIMOTE_BTN_LEFT)
		x360_pad.report.wButtons |= XUSB_GAMEPAD_DPAD_DOWN;
	if (wiiu_btns & WIIU_GAMEPAD_BTN_UP || wiimote_btns & WIIMOTE_BTN_RIGHT)
		x360_pad.report.wButtons |= XUSB_GAMEPAD_DPAD_UP;
	if (wiiu_btns & WIIU_GAMEPAD_BTN_LEFT || wiimote_btns & WIIMOTE_BTN_UP)
		x360_pad.report.wButtons |= XUSB_GAMEPAD_DPAD_LEFT;
	if (wiiu_btns & WIIU_GAMEPAD_BTN_RIGHT || wiimote_btns & WIIMOTE_BTN_DOWN)
		x360_pad.report.wButtons |= XUSB_GAMEPAD_DPAD_RIGHT;

	if (wiiu_btns & WIIU_GAMEPAD_BTN_PLUS || wiimote_btns & WIIMOTE_BTN_PLUS)
		x360_pad.report.wButtons |= XUSB_GAMEPAD_START;
	if (wiiu_btns & WIIU_GAMEPAD_BTN_MINUS || wiimote_btns & WIIMOTE_BTN_MINUS)
		x360_pad.report.wButtons |= XUSB_GAMEPAD_BACK;
	if (wiiu_btns & WIIU_GAMEPAD_BTN_L)
		x360_pad.report.wButtons |= XUSB_GAMEPAD_LEFT_SHOULDER;
	if (wiiu_btns & WIIU_GAMEPAD_BTN_R)
		x360_pad.report.wButtons |= XUSB_GAMEPAD_RIGHT_SHOULDER;

	if (wiiu_btns & WIIU_GAMEPAD_BTN_STICK_R)
		x360_pad.report.wButtons |= XUSB_GAMEPAD_RIGHT_THUMB;
	if (wiiu_btns & WIIU_GAMEPAD_BTN_STICK_L)
		x360_pad.report.wButtons |= XUSB_GAMEPAD_LEFT_THUMB;

	x360_pad.report.bLeftTrigger = (wiiu_btns&WIIU_GAMEPAD_BTN_ZL) ? 0xFF : 0x00;
	x360_pad.report.bRightTrigger = (wiiu_btns&WIIU_GAMEPAD_BTN_ZR) ? 0xFF : 0x00;
	x360_pad.report.sThumbLX = pack->gamepad.lsx;
	x360_pad.report.sThumbLY = pack->gamepad.lsy;
	x360_pad.report.sThumbRX = pack->gamepad.rsx;
	x360_pad.report.sThumbRY = pack->gamepad.rsy;

	VIGEM_ERROR ret = vigem_target_x360_update(
		x360_pad.client,
		x360_pad.target,
		x360_pad.report
	);

	if (ret != VIGEM_ERROR_NONE) {
		log_info("Error on vigem target update: %X\n", ret);
	}
}

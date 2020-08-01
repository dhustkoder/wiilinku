#include <vpad/input.h>
#include <padscore/kpad.h>
#include <padscore/wpad.h>
#include <stdint.h>
#include "packets.h"
#include "inputman.h"
#include "log.h"


static VPADReadError verror;
static VPADStatus vpad;
static KPADStatus kpads[4];
static volatile uint8_t kpad_connected[4] = { 0, 0, 0, 0 };
static volatile uint8_t wiimote_flags = 0;
static struct input_packet last_input;



void wpad_connection_callback(WPADChan chan, int32_t status)
{
	log_info(
		"UPDATED WPAD[%d] = STATUS %d",
		(int)chan,
		(int)status
	);

	kpad_connected[chan] = status == 0 ? 1 : 0;
	
	int kchan_flags[4] = {
		[WPAD_CHAN_0] = INPUT_PACKET_FLAG_WIIMOTE_0,
		[WPAD_CHAN_1] = INPUT_PACKET_FLAG_WIIMOTE_1,
		[WPAD_CHAN_2] = INPUT_PACKET_FLAG_WIIMOTE_2,
		[WPAD_CHAN_3] = INPUT_PACKET_FLAG_WIIMOTE_3
	};

	if (!kpad_connected[chan]) {
		memset(&kpads[chan], 0, sizeof kpads[chan]);
		wiimote_flags &= ~kchan_flags[chan];
	} else {
		wiimote_flags |= kchan_flags[chan];
	}
}

void inputman_init(void)
{
	VPADInit();
	WPADInit();
	WPADEnableURCC(1);

	WPADSetConnectCallback(WPAD_CHAN_0, wpad_connection_callback);
	WPADSetConnectCallback(WPAD_CHAN_1, wpad_connection_callback);
	WPADSetConnectCallback(WPAD_CHAN_2, wpad_connection_callback);
	WPADSetConnectCallback(WPAD_CHAN_3, wpad_connection_callback);

	memset(kpads, 0, sizeof kpads);
	memset(&vpad, 0, sizeof vpad);
}

void inputman_term(void)
{
	WPADShutdown();
	VPADShutdown();
}

void inputman_update(
	const struct input_feedback_packet* feedback,
	struct input_packet* input
)
{
	assert(input != NULL);

	input->flags = wiimote_flags|INPUT_PACKET_FLAG_GAMEPAD;
	
	VPADRead(VPAD_CHAN_0, &vpad, 1, &verror);

	for (int i = 0; i < 4; ++i) {
		if (kpad_connected[i]) {
			WPADRead(i, &kpads[i]);
		}
	}

	const VPADVec2D ls = vpad.leftStick;
	const VPADVec2D rs = vpad.rightStick;
	
	input->gamepad.btns = vpad.hold;
	input->gamepad.lsx = ls.x * INT16_MAX;
	input->gamepad.lsy = ls.y * INT16_MAX;
	input->gamepad.rsx = rs.x * INT16_MAX;
	input->gamepad.rsy = rs.y * INT16_MAX;

	for (int i = 0; i < 4; ++i) {
		if (kpad_connected[i]) {
			input->wiimotes[i].btns = kpads[i].hold;
		}
	}

	memcpy(&last_input, input, sizeof last_input);
}

void inputman_fetch(struct input_packet* input)
{
	memcpy(input, &last_input, sizeof *input);
}





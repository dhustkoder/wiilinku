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

	
void wpad_connection_callback(WPADChan chan, int32_t status)
{
	log_info(
		"UPDATED WPAD[%d] = STATUS %d",
		(int)chan,
		(int)status
	);

	kpad_connected[chan] = status == 0 ? 1 : 0;
	if (!kpad_connected[chan])
		memset(&kpads[chan], 0, sizeof kpads[chan]);
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

void inputman_update(struct input_packet* pack)
{
	VPADRead(VPAD_CHAN_0, &vpad, 1, &verror);

	for (int i = 0; i < 4; ++i) {
		if (kpad_connected[i]) {
			WPADRead(i, &kpads[i]);
		}
	}

	inputman_fetch(pack);
}

void inputman_fetch(struct input_packet* pack)
{
	assert(pack != NULL);

	const VPADVec2D ls = vpad.leftStick;
	const VPADVec2D rs = vpad.rightStick;
	
	pack->gamepad.btns = vpad.hold;
	pack->gamepad.lsx = ls.x * INT16_MAX;
	pack->gamepad.lsy = ls.y * INT16_MAX;
	pack->gamepad.rsx = rs.x * INT16_MAX;
	pack->gamepad.rsy = rs.y * INT16_MAX;

	for (int i = 0; i < 4; ++i) {
		if (kpad_connected[i]) {
			pack->wiimotes[i].btns = kpads[i].hold;
		}
	}

}





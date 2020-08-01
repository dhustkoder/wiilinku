#include <stdlib.h>
#include <vpad/input.h>
#include <padscore/kpad.h>
#include <padscore/wpad.h>
#include <coreinit/thread.h>
#include <stdint.h>
#include "packets.h"
#include "connection.h"
#include "input.h"
#include "log.h"


static VPADReadError verror;
static VPADStatus vpad;
static KPADStatus kpads[4];
static volatile uint8_t kpad_connected[4] = { 0, 0, 0, 0 };
static volatile uint8_t wiimote_flags = 0;
static struct input_packet last_input;



static void wpad_connection_callback(WPADChan chan, int32_t status)
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

static void input_update(void)
{
	last_input.flags = wiimote_flags|INPUT_PACKET_FLAG_GAMEPAD;
	
	VPADRead(VPAD_CHAN_0, &vpad, 1, &verror);

	for (int i = 0; i < 4; ++i) {
		if (kpad_connected[i]) {
			WPADRead(i, &kpads[i]);
		}
	}

	const VPADVec2D ls = vpad.leftStick;
	const VPADVec2D rs = vpad.rightStick;
	
	last_input.gamepad.btns = vpad.hold;
	last_input.gamepad.lsx = ls.x * INT16_MAX;
	last_input.gamepad.lsy = ls.y * INT16_MAX;
	last_input.gamepad.rsx = rs.x * INT16_MAX;
	last_input.gamepad.rsy = rs.y * INT16_MAX;

	for (int i = 0; i < 4; ++i) {
		if (kpad_connected[i]) {
			last_input.wiimotes[i].btns = kpads[i].hold;
		}
	}
}

static int input_updater_thread_main(int argc, const char** argv)
{
	struct input_feedback_packet feedback;

	for (;;) {
		input_update();

		if (last_input.gamepad.btns&WIIU_GAMEPAD_BTN_HOME)
			goto Lexit;

		connection_send_input_packet(&last_input);

		if (connection_receive_input_feedback_packet(&feedback)) {
			log_debug("recv feedback: %.2X", (unsigned) feedback.placeholder);
		}

	}

Lexit:
	return EXIT_SUCCESS;
}


void input_init(void)
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

	OSRunThread(OSGetDefaultThread(0), input_updater_thread_main, 0, NULL);
}

void input_term(void)
{
	WPADShutdown();
	VPADShutdown();
}



void input_fetch(struct input_packet* input)
{
	memcpy(input, &last_input, sizeof *input);
}





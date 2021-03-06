#ifndef WIILINKU_PACKETS_H_
#define WIILINKU_PACKETS_H_
#include "base_defs.h"


#define PING_PACKET_PORT           (7171)
#define INPUT_PACKET_PORT          (7172)
#define INPUT_FEEDBACK_PACKET_PORT (7173)
#define PING_INTERVAL_SEC          (6)



enum wiiu_gamepad_btn {
	WIIU_GAMEPAD_BTN_A       = 0x8000,
	WIIU_GAMEPAD_BTN_B       = 0x4000,
	WIIU_GAMEPAD_BTN_X       = 0x2000,
	WIIU_GAMEPAD_BTN_Y       = 0x1000,
	WIIU_GAMEPAD_BTN_LEFT    = 0x0800,
	WIIU_GAMEPAD_BTN_RIGHT   = 0x0400,
	WIIU_GAMEPAD_BTN_UP      = 0x0200,
	WIIU_GAMEPAD_BTN_DOWN    = 0x0100,
	WIIU_GAMEPAD_BTN_ZL      = 0x0080,
	WIIU_GAMEPAD_BTN_ZR      = 0x0040,
	WIIU_GAMEPAD_BTN_L       = 0x0020,
	WIIU_GAMEPAD_BTN_R       = 0x0010,
	WIIU_GAMEPAD_BTN_PLUS    = 0x0008,
	WIIU_GAMEPAD_BTN_MINUS   = 0x0004,
	WIIU_GAMEPAD_BTN_STICK_R = 0x00020000,
	WIIU_GAMEPAD_BTN_STICK_L = 0x00040000,
	//! The HOME button.
	WIIU_GAMEPAD_BTN_HOME              = 0x0002,
	//! The SYNC button.
	WIIU_GAMEPAD_BTN_SYNC              = 0x0001,
	//! The TV button.
	WIIU_GAMEPAD_BTN_TV                = 0x00010000,
	//! The emulated left button on the right stick.
	WIIU_GAMEPAD_STICK_R_EMULATION_LEFT   = 0x04000000,
	//! The emulated right button on the right stick.
	WIIU_GAMEPAD_STICK_R_EMULATION_RIGHT  = 0x02000000,
	//! The emulated up button on the right stick.
	WIIU_GAMEPAD_STICK_R_EMULATION_UP     = 0x01000000,
	//! The emulated down button on the right stick.
	WIIU_GAMEPAD_STICK_R_EMULATION_DOWN   = 0x00800000,
	//! The emulated left button on the left stick.
	WIIU_GAMEPAD_STICK_L_EMULATION_LEFT   = 0x40000000,
	//! The emulated right button on the left stick.
	WIIU_GAMEPAD_STICK_L_EMULATION_RIGHT  = 0x20000000,
	//! The emulated up button on the left stick.
	WIIU_GAMEPAD_STICK_L_EMULATION_UP     = 0x10000000,
	//! The emulated down button on the left stick.
	WIIU_GAMEPAD_STICK_L_EMULATION_DOWN   = 0x08000000,
};

enum wiimote_btn {
	//! The left button of the D-pad.
	WIIMOTE_BTN_LEFT                    = 0x00010000,
	//! The right button of the D-pad.
	WIIMOTE_BTN_RIGHT                   = 0x00020000,
	//! The down button of the D-pad.
	WIIMOTE_BTN_DOWN                    = 0x00040000,
	//! The up button of the D-pad.
	WIIMOTE_BTN_UP                      = 0x00080000,
	//! The + button.
	WIIMOTE_BTN_PLUS                    = 0x00100000,
	//! The 2 button.
	WIIMOTE_BTN_2                       = 0x01000000,
	//! The 1 button.
	WIIMOTE_BTN_1                       = 0x02000000,
	//! The B button.
	WIIMOTE_BTN_B                       = 0x04000000,
	//! The A button.
	WIIMOTE_BTN_A                       = 0x08000000,
	//! The - button.
	WIIMOTE_BTN_MINUS                   = 0x10000000,
	//! The Z button on the Nunchuk extension.
	WIIMOTE_BTN_Z                       = 0x20000000,
	//! The C button on the Nunchuk extension.
	WIIMOTE_BTN_C                       = 0x40000000,
	//! The HOME button.
	WIIMOTE_BTN_HOME                    = 0x80000000,
};


struct wiiu_gamepad {
	u32 btns;
	s16 lsx;
	s16 lsy;
	s16 rsx;
	s16 rsy;
};


struct wiimote {
	u32 btns;
};

typedef uint8_t input_packet_flags_t;
enum INPUT_PACKET_FLAGS {
	INPUT_PACKET_FLAG_GAMEPAD   = 0x01,
	INPUT_PACKET_FLAG_WIIMOTE_0 = 0x02,
	INPUT_PACKET_FLAG_WIIMOTE_1 = 0x04,
	INPUT_PACKET_FLAG_WIIMOTE_2 = 0x08,
	INPUT_PACKET_FLAG_WIIMOTE_3 = 0x10
};

struct input_packet {
	input_packet_flags_t flags;
	struct wiiu_gamepad gamepad;
	struct wiimote wiimotes[4];
};

struct input_feedback_packet {
	uint8_t placeholder;
};


static inline void input_packet_reorder(struct input_packet* p)
{
	if (p->flags&INPUT_PACKET_FLAG_GAMEPAD) {
		const u32 btns = p->gamepad.btns;
		const s16 rsx = p->gamepad.rsx;
		const s16 rsy = p->gamepad.rsy;
		const s16 lsx = p->gamepad.lsx;
		const s16 lsy = p->gamepad.lsy;

		p->gamepad.btns = BSWAP_32(btns);

		p->gamepad.rsx = BSWAP_16(rsx);

		p->gamepad.rsy = BSWAP_16(rsy);

		p->gamepad.lsx = BSWAP_16(lsx);

		p->gamepad.lsy = BSWAP_16(lsy);
	}

	const uint8_t wiimote_flags[] = {
		INPUT_PACKET_FLAG_WIIMOTE_0,
		INPUT_PACKET_FLAG_WIIMOTE_1,
		INPUT_PACKET_FLAG_WIIMOTE_2,
		INPUT_PACKET_FLAG_WIIMOTE_3
	};


	for (int i = 0; i < 4; ++i) {
		if (p->flags&wiimote_flags[i]) {
			const u32 btns = p->wiimotes[i].btns;
			p->wiimotes[i].btns = BSWAP_32(btns); 
		}
	}
}




#endif

#ifndef WIIUPCX_X360EMU_H_
#define WIIUPCX_X360EMU_H_
#include <stdint.h>
#include "netn.h"

enum wiiu_gamepad_button {
	WIIU_GAMEPAD_BUTTON_A     = 0x8000,
	WIIU_GAMEPAD_BUTTON_B     = 0x4000,
	WIIU_GAMEPAD_BUTTON_X     = 0x2000,
	WIIU_GAMEPAD_BUTTON_Y     = 0x1000,
	WIIU_GAMEPAD_BUTTON_LEFT  = 0x0800,
	WIIU_GAMEPAD_BUTTON_RIGHT = 0x0400,
	WIIU_GAMEPAD_BUTTON_UP    = 0x0200,
	WIIU_GAMEPAD_BUTTON_DOWN  = 0x0100,
	WIIU_GAMEPAD_BUTTON_ZL    = 0x0080,
	WIIU_GAMEPAD_BUTTON_ZR    = 0x0040,
	WIIU_GAMEPAD_BUTTON_L     = 0x0020,
	WIIU_GAMEPAD_BUTTON_R     = 0x0010,
	WIIU_GAMEPAD_BUTTON_PLUS  = 0x0008,
	WIIU_GAMEPAD_BUTTON_MINUS = 0x0004,
	WIIU_GAMEPAD_BUTTON_STICK_R = 0x00020000,
	WIIU_GAMEPAD_BUTTON_STICK_L = 0x00040000
};

enum wiimote_button
{
   //! The left button of the D-pad.
   WIIMOTE_BUTTON_LEFT                    = 0x0001,
   //! The right button of the D-pad.
   WIIMOTE_BUTTON_RIGHT                   = 0x0002,
   //! The down button of the D-pad.
   WIIMOTE_BUTTON_DOWN                    = 0x0004,
   //! The up button of the D-pad.
   WIIMOTE_BUTTON_UP                      = 0x0008,
   //! The + button.
   WIIMOTE_BUTTON_PLUS                    = 0x0010,
   //! The 2 button.
   WIIMOTE_BUTTON_2                       = 0x0100,
   //! The 1 button.
   WIIMOTE_BUTTON_1                       = 0x0200,
   //! The B button.
   WIIMOTE_BUTTON_B                       = 0x0400,
   //! The A button.
   WIIMOTE_BUTTON_A                       = 0x0800,
   //! The - button.
   WIIMOTE_BUTTON_MINUS                   = 0x1000,
   //! The Z button on the Nunchuk extension.
   WIIMOTE_BUTTON_Z                       = 0x2000,
   //! The C button on the Nunchuk extension.
   WIIMOTE_BUTTON_C                       = 0x4000,
   //! The HOME button.
   WIIMOTE_BUTTON_HOME                    = 0x8000,
};


int x360emu_init(void);
void x360emu_term(void);
int x360emu_update(struct netn_joy_packet* jpkt);

#endif
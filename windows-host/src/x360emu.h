#ifndef WIIUPCX_X360EMU_H_
#define WIIUPCX_X360EMU_H_
#include <stdint.h>


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
	float x, y;
};


int x360emu_init(void);
void x360emu_term(void);
int x360emu_update(uint32_t wiiu_btns, struct vec2 ls, struct vec2 rs);

#endif
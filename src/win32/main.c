#include <stdint.h>
#include <limits.h>
#include <stdio.h>
#include "gui.h"
#include "log.h"
#include "x360emu.h"

#define NETN_IMPLEMENTATION
#include "netn.h"




static int init_platform(HINSTANCE hins, int ncmd)
{
	if (log_init())
		return 1;

	if (gui_init(hins, ncmd))
		return 1;
	
	if (x360emu_init())
		return 1;

	if (netn_init())
		return 1;

	return 0;
}

static void terminate_platform(void)
{
	netn_term();
	x360emu_term();
	gui_term();
	log_term();
}

int CALLBACK WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR lpCmdLine,
                     const int nCmdShow)
{
	((void)hPrevInstance);
	((void)lpCmdLine);

	if (init_platform(hInstance, nCmdShow))
		return 1;
		
	
	struct netn_joy_packet jpkt;

	for (;;) {
		if (gui_win_update()) {
			break;
		}
/*
		if (netn_joy_update(&jpkt)) {
			log_info("ERROR RECV INPUT");
		}

		log_info(
			"GAMEPAD: %.8X %.4X %.4X %.4X %.4X"
			"WIIMOTE: %.8X",
			jpkt.gamepad.btns, jpkt.gamepad.lsx, jpkt.gamepad.lsy, jpkt.gamepad.rsx, jpkt.gamepad.rsy,
			jpkt.wiimote.btns
		);

		x360emu_update(&jpkt);
*/
	}


	terminate_platform();

	return 0;
}

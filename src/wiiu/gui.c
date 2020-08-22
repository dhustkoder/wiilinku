#include <string.h>
#include "log.h"
#include "gui.h"
#include "zui.h"
#include "input.h"
#include "connection.h"
#include "render.h"



struct gui_text {
	zui_obj_id_t id;
	struct vec2i origin;
	const char* str;
};

static struct rgb24 zui_fb[ZUI_WIDTH * ZUI_HEIGHT];


char ipbuf[24] =  "192.168.000.000\0";
char ipcurs[24] = "^              \0";
const int ip_cur_pos_table[12] = { 0, 1, 2, 4, 5, 6, 8, 9, 10, 12, 13, 14 };
int ip_cur_pos = 0;

VPADStatus vpad;

struct gui_text gui_header_txts[] = {
	{
		.origin = { .x = ZUI_WIDTH / 2, .y = 22 },
		.str = "WiiLinkU " WLU_VERSION_STR
	},
	{
		.origin = { .x = ZUI_WIDTH / 2, .y = 38 },
		.str = ipbuf
	},
	{
		.origin = { .x = ZUI_WIDTH / 2, .y = 47 },
		.str = ipcurs
	},
	{
		.origin = { .x = ZUI_WIDTH / 2, .y = 86 },
		.str = "Connection Status: Not Connected"
	},
	{
		.origin = { .x = ZUI_WIDTH / 2, .y = ZUI_HEIGHT / 2},
		.str = "SELECT IP: UP DOWN LEFT RIGHT\n"
		       "CONNECT:   +\n"
		       "EXIT:      HOME BUTTON\n"
	},
	{
		.origin = { .x = ZUI_WIDTH / 2, .y = (ZUI_HEIGHT / 2) + 50},
		.str = "              !!  Remember  !!              \n"
		       "   You need the Desktop App to use WiiLinkU \n"
		       "        Get it at: https://git.io/JJAo9     "
	}
};




bool gui_init(void)
{
	render_init();
	zui_init();

	for (int i = 0; i < (sizeof(gui_header_txts)/sizeof(gui_header_txts[0])); ++i) {
		gui_header_txts[i].id = zui_text_create(gui_header_txts[i].origin);
		zui_text_set(gui_header_txts[i].id, gui_header_txts[i].str);
		zui_text_draw(gui_header_txts[i].id);
	}

	memset(&vpad, 0, sizeof(VPADStatus));

	return true;
}

void gui_term(void)
{
	zui_term();
	render_term();
}

bool gui_update(const char** ip)
{
	input_fetch_vpad(&vpad);

	if (vpad.trigger&VPAD_BUTTON_HOME)
		return false;

	if (!connection_is_connected()) {

		ipcurs[ip_cur_pos_table[ip_cur_pos]] = ' ';

		if (vpad.trigger&VPAD_BUTTON_RIGHT && ip_cur_pos < 11) {
			++ip_cur_pos;
		} else if (vpad.trigger&VPAD_BUTTON_LEFT && ip_cur_pos > 0) {
			--ip_cur_pos;
		}

		ipcurs[ip_cur_pos_table[ip_cur_pos]] = '^';


		if (vpad.trigger&VPAD_BUTTON_UP && ipbuf[ip_cur_pos_table[ip_cur_pos]] < '9') {
			++ipbuf[ip_cur_pos_table[ip_cur_pos]];
		} else if (vpad.trigger&VPAD_BUTTON_DOWN && ipbuf[ip_cur_pos_table[ip_cur_pos]] > '0') {
			--ipbuf[ip_cur_pos_table[ip_cur_pos]];
		}

		
		zui_text_set(gui_header_txts[2].id, ipcurs);
		zui_text_draw(gui_header_txts[2].id);

		zui_text_set(gui_header_txts[1].id, ipbuf);
		zui_text_draw(gui_header_txts[1].id);

		if (vpad.trigger&VPAD_BUTTON_PLUS && *ip == NULL) {
			*ip = ipbuf;
			zui_text_set(gui_header_txts[3].id, "Connection Status: Connecting.......");
			zui_text_draw(gui_header_txts[3].id);
		} else if (*ip == NULL) {
			zui_text_set(gui_header_txts[3].id, "Connection Status: Not Connected");
			zui_text_draw(gui_header_txts[3].id);
		}

	} else {
		zui_text_set(gui_header_txts[3].id, "Connection Status: Connected");
		zui_text_draw(gui_header_txts[3].id);
	}

	if (zui_update()) {
		zui_render(zui_fb);
		render_write_texture(zui_fb);
	}


	render_flush_n_wait_vsync();

	return true;
}



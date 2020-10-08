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
static char ipbuf[24] =  "192.168.000.000\0";
static char ipcurs[24] = "^              \0";
static const int ip_cur_pos_table[12] = { 0, 1, 2, 4, 5, 6, 8, 9, 10, 12, 13, 14 };
static int ip_cur_pos = 0;

VPADStatus vpad;


enum gui_txt_idx {
	GUI_TXT_IDX_TITLE,
	GUI_TXT_IDX_CONTROLS,
	GUI_TXT_IDX_IPBUF,
	GUI_TXT_IDX_IPCURS,
	GUI_TXT_IDX_CONN_STAT,
	GUI_TXT_IDX_HOSTAPP_OBS
};

struct gui_text gui_txts[] = {
	[GUI_TXT_IDX_TITLE] = {
		.origin = { .x = ZUI_WIDTH / 2, .y = 22 },
		.str = "WiiLinkU " WLU_VERSION_STR
	},

	[GUI_TXT_IDX_CONTROLS] = {
		.origin = { .x = ZUI_WIDTH / 2, .y = 48 },
		.str = "Use the DPAD to enter the Host IP Address"
	},

	[GUI_TXT_IDX_IPBUF] = {
		.origin = { .x = ZUI_WIDTH / 2, .y = 62 },
		.str = ipbuf
	},

	[GUI_TXT_IDX_IPCURS] = {
		.origin = { .x = ZUI_WIDTH / 2, .y = 71 },
		.str = ipcurs
	},

	[GUI_TXT_IDX_CONN_STAT] = {
		.origin = { .x = ZUI_WIDTH / 2, .y = 86 },
		.str = "Connection Status: Not Connected"
	},
	
	[GUI_TXT_IDX_HOSTAPP_OBS] = {
		.origin = { .x = ZUI_WIDTH / 2, .y = (ZUI_HEIGHT / 2) + 94 },
		.str = "              !!  Remember  !!              \n"
		       "   You need the Desktop App to use WiiLinkU \n"
		       "        Get it at: https://git.io/JJAo9     "
	}
};




bool gui_init(void)
{
	render_init();
	zui_init();

	for (int i = 0; i < (sizeof(gui_txts)/sizeof(gui_txts[0])); ++i) {
		gui_txts[i].id = zui_text_create(gui_txts[i].origin);
		zui_text_set(gui_txts[i].id, gui_txts[i].str);
		zui_text_draw(gui_txts[i].id);
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

	if (vpad.tpNormal.touched) {
		log_debug(
			"TOUCH INFO: \n"
			"X : %d\n"
			"Y : %d\n",
			(int)vpad.tpNormal.x,
			(int)vpad.tpNormal.y
		);
		render_switch_drc();
	}

	if (vpad.trigger&VPAD_BUTTON_HOME)
		return false;

	if (!connection_is_connected()) {

		if (vpad.trigger&(VPAD_BUTTON_UP|VPAD_BUTTON_DOWN)) {
			if (vpad.trigger&VPAD_BUTTON_UP && ipbuf[ip_cur_pos_table[ip_cur_pos]] < '9') {
				++ipbuf[ip_cur_pos_table[ip_cur_pos]];
			} else if (vpad.trigger&VPAD_BUTTON_DOWN && ipbuf[ip_cur_pos_table[ip_cur_pos]] > '0') {
				--ipbuf[ip_cur_pos_table[ip_cur_pos]];
			}
			zui_text_set(gui_txts[GUI_TXT_IDX_IPBUF].id, ipbuf);
			zui_text_draw(gui_txts[GUI_TXT_IDX_IPBUF].id);
		}

		if (vpad.trigger&(VPAD_BUTTON_RIGHT|VPAD_BUTTON_LEFT)) {
			ipcurs[ip_cur_pos_table[ip_cur_pos]] = ' ';

			if (vpad.trigger&VPAD_BUTTON_RIGHT && ip_cur_pos < 11) {
				++ip_cur_pos;
			} else if (vpad.trigger&VPAD_BUTTON_LEFT && ip_cur_pos > 0) {
				--ip_cur_pos;
			}

			ipcurs[ip_cur_pos_table[ip_cur_pos]] = '^';
			zui_text_set(gui_txts[GUI_TXT_IDX_IPCURS].id, ipcurs);
			zui_text_draw(gui_txts[GUI_TXT_IDX_IPCURS].id);
		}


		if (vpad.trigger&VPAD_BUTTON_PLUS && *ip == NULL) {
			*ip = ipbuf;
			zui_text_set(gui_txts[GUI_TXT_IDX_CONN_STAT].id, "Connection Status: Connecting...");
			zui_text_draw(gui_txts[GUI_TXT_IDX_CONN_STAT].id);
		} else if (*ip == NULL) {
			zui_text_set(gui_txts[GUI_TXT_IDX_CONN_STAT].id, "Connection Status: Not Connected");
			zui_text_draw(gui_txts[GUI_TXT_IDX_CONN_STAT].id);
		}

		zui_text_draw(gui_txts[GUI_TXT_IDX_HOSTAPP_OBS].id);

	} else {
		zui_text_erase(gui_txts[GUI_TXT_IDX_HOSTAPP_OBS].id);
		zui_text_set(gui_txts[GUI_TXT_IDX_CONN_STAT].id, "Connection Status: Connected");
		zui_text_draw(gui_txts[GUI_TXT_IDX_CONN_STAT].id);
	}

	if (zui_update()) {
		zui_render(zui_fb);
		render_write_texture(zui_fb);
	}


	render_flush_n_wait_vsync();

	return true;
}



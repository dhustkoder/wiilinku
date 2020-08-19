#include <gfd.h>
#include <gx2/draw.h>
#include <gx2/shaders.h>
#include <gx2/swap.h>
#include <gx2/mem.h>
#include <gx2/registers.h>
#include <gx2r/draw.h>
#include <gx2r/buffer.h>
#include <string.h>
#include <stdio.h>
#include <whb/gfx.h>

#include <coreinit/memdefaultheap.h>
#include <coreinit/systeminfo.h>
#include <coreinit/thread.h>
#include <coreinit/time.h>

#include <stdlib.h>
#include "gui.h"
#include "texture_shader.h"
#include "zui.h"
#include "input.h"
#include "utils.h"
#include "connection.h"
#include "log.h"


static const float position_vb[] = {
	-1.0f, -1.0f,
	1.0f, -1.0f,
	1.0f,  1.0f,
	-1.0f,  1.0f,
};

static const float tex_coord_vb[] = {
	0.0f, 1.0f,
	1.0f, 1.0f,
	1.0f, 0.0f,
	0.0f, 0.0f,
};

GX2Texture texture = {0};
GX2RBuffer position_buffer = { 0 };
GX2RBuffer tex_coord_buffer = { 0 };
WHBGfxShaderGroup group = { 0 };
GX2Sampler sampler;

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


static void gui_render_flush(void)
{	
	WHBGfxBeginRender();

	WHBGfxBeginRenderTV();
	
	WHBGfxClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	GX2SetFetchShader(&group.fetchShader);
	GX2SetVertexShader(group.vertexShader);
	GX2SetPixelShader(group.pixelShader);
	GX2RSetAttributeBuffer(&position_buffer, 0, position_buffer.elemSize, 0);
	GX2RSetAttributeBuffer(&tex_coord_buffer, 1, tex_coord_buffer.elemSize, 0);

	GX2SetPixelTexture(&texture, group.pixelShader->samplerVars[0].location);
	GX2SetPixelSampler(&sampler, group.pixelShader->samplerVars[0].location);
	GX2DrawEx(GX2_PRIMITIVE_MODE_QUADS, 4, 0, 1);
	
	WHBGfxFinishRenderTV();

	WHBGfxBeginRenderDRC();

	WHBGfxClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	GX2SetFetchShader(&group.fetchShader);
	GX2SetVertexShader(group.vertexShader);
	GX2SetPixelShader(group.pixelShader);
	GX2RSetAttributeBuffer(&position_buffer, 0, position_buffer.elemSize, 0);
	GX2RSetAttributeBuffer(&tex_coord_buffer, 1, tex_coord_buffer.elemSize, 0);

	GX2SetPixelTexture(&texture, group.pixelShader->samplerVars[0].location);
	GX2SetPixelSampler(&sampler, group.pixelShader->samplerVars[0].location);
	GX2DrawEx(GX2_PRIMITIVE_MODE_QUADS, 4, 0, 1);

	WHBGfxFinishRenderDRC();

	WHBGfxFinishRender();
}


bool gui_init(void)
{
	void *buffer = NULL;
	const char *gshFileData = NULL;
	WHBGfxInit();
	zui_init();


	gshFileData = texture_shader_gsh; //texture_shader.gsh;

	if (!WHBGfxLoadGFDShaderGroup(&group, 0, gshFileData)) {
		gui_term();
		return false;
	}

	WHBGfxInitShaderAttribute(&group, "position", 0, 0, GX2_ATTRIB_FORMAT_FLOAT_32_32);
	WHBGfxInitShaderAttribute(&group, "tex_coord_in", 1, 0, GX2_ATTRIB_FORMAT_FLOAT_32_32);
	WHBGfxInitFetchShader(&group);

	/* set up Attribute Buffers */


	// Set vertex position
	position_buffer.flags = GX2R_RESOURCE_BIND_VERTEX_BUFFER |
		GX2R_RESOURCE_USAGE_CPU_READ |
		GX2R_RESOURCE_USAGE_CPU_WRITE |
		GX2R_RESOURCE_USAGE_GPU_READ;
	position_buffer.elemSize = 2 * 4;
	position_buffer.elemCount = 4;
	GX2RCreateBuffer(&position_buffer);
	buffer = GX2RLockBufferEx(&position_buffer, 0);
	memcpy(buffer, position_vb, position_buffer.elemSize * position_buffer.elemCount);
	GX2RUnlockBufferEx(&position_buffer, 0);

	// Set vertex colour
	tex_coord_buffer.flags = GX2R_RESOURCE_BIND_VERTEX_BUFFER |
		GX2R_RESOURCE_USAGE_CPU_READ |
		GX2R_RESOURCE_USAGE_CPU_WRITE |
		GX2R_RESOURCE_USAGE_GPU_READ;
	tex_coord_buffer.elemSize = 2 * 4;
	tex_coord_buffer.elemCount = 4;
	GX2RCreateBuffer(&tex_coord_buffer);
	buffer = GX2RLockBufferEx(&tex_coord_buffer, 0);
	memcpy(buffer, tex_coord_vb, tex_coord_buffer.elemSize * tex_coord_buffer.elemCount);
	GX2RUnlockBufferEx(&tex_coord_buffer, 0);

	/* create a texture */
	int width = ZUI_WIDTH;
	int height = ZUI_HEIGHT;
	texture.surface.width    = width;
	texture.surface.height   = height;
	texture.surface.depth    = 1;
	texture.surface.dim      = GX2_SURFACE_DIM_TEXTURE_2D;
	texture.surface.format   = GX2_SURFACE_FORMAT_UNORM_R8_G8_B8_A8;
	texture.surface.tileMode = GX2_TILE_MODE_LINEAR_ALIGNED;
	texture.viewNumSlices    = 1;
	texture.compMap          = 0x00010203;
	GX2CalcSurfaceSizeAndAlignment(&texture.surface);
	GX2InitTextureRegs(&texture);

	texture.surface.image = MEMAllocFromDefaultHeapEx(texture.surface.imageSize, texture.surface.alignment);

	log_debug("texture width: %d", texture.surface.width);
	log_debug("texture height: %d", texture.surface.height);
	log_debug("texture pitch: %d", texture.surface.pitch);

	GX2InitSampler(&sampler, GX2_TEX_CLAMP_MODE_CLAMP, GX2_TEX_XY_FILTER_MODE_LINEAR);

	for (int i = 0; i < (sizeof(gui_header_txts)/sizeof(gui_header_txts[0])); ++i) {
		gui_header_txts[i].id = zui_text_create(gui_header_txts[i].origin);
		zui_text_set(gui_header_txts[i].id, gui_header_txts[i].str);
		zui_text_draw(gui_header_txts[i].id);
	}


	GX2SetSwapInterval(3);
	memset(&vpad, 0, sizeof(VPADStatus));

	return true;
}

void gui_term(void)
{
	zui_term();
	GX2RDestroyBufferEx(&position_buffer, 0);
	GX2RDestroyBufferEx(&tex_coord_buffer, 0);
	WHBGfxShutdown();
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
		u32* dst = (u32*)texture.surface.image;
		struct rgb24* src = zui_fb;
		for (u32 i = 0; i < ZUI_HEIGHT; ++i) {
			for (u32 j = 0; j < ZUI_WIDTH; ++j) {
				dst[j] = (src->r<<24)|(src->g<<16)|(src->b<<8);
				++src;
			}
			dst += texture.surface.pitch;
		}
		GX2Invalidate(GX2_INVALIDATE_MODE_CPU_TEXTURE, texture.surface.image, texture.surface.imageSize);
	}


	gui_render_flush();

	return true;
}



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
#include "zui.h"
#include "texture_shader.h"
#include "render.h"
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


bool render_init(void)
{
	void *buffer = NULL;
	const char *gshFileData = NULL;
	WHBGfxInit();

	gshFileData = texture_shader_gsh; //texture_shader.gsh;

	if (!WHBGfxLoadGFDShaderGroup(&group, 0, gshFileData)) {
		render_term();
		return false;
	}

	WHBGfxInitShaderAttribute(&group, "position", 0, 0, GX2_ATTRIB_FORMAT_FLOAT_32_32);
	WHBGfxInitShaderAttribute(&group, "tex_coord_in", 1, 0, GX2_ATTRIB_FORMAT_FLOAT_32_32);
	WHBGfxInitFetchShader(&group);

	/* set up Attribute Buffers */


	// Set vertex position
	position_buffer.flags = GX2R_RESOURCE_BIND_VERTEX_BUFFER |
	                        GX2R_RESOURCE_USAGE_CPU_READ     |
	                        GX2R_RESOURCE_USAGE_CPU_WRITE    |
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
	GX2SetSwapInterval(3);

	return true;
}

void render_term(void)
{
	GX2RDestroyBufferEx(&position_buffer, 0);
	GX2RDestroyBufferEx(&tex_coord_buffer, 0);
	WHBGfxShutdown();
}

void render_write_texture(void *data)
{
	u32* dst = (u32*)texture.surface.image;
	struct rgb24* src = data;
	for (u32 i = 0; i < ZUI_HEIGHT; ++i) {
		for (u32 j = 0; j < ZUI_WIDTH; ++j) {
			dst[j] = (src->r<<24)|(src->g<<16)|(src->b<<8);
			++src;
		}
		dst += texture.surface.pitch;
	}
	GX2Invalidate(GX2_INVALIDATE_MODE_CPU_TEXTURE, texture.surface.image, texture.surface.imageSize);
}

void render_flush_n_wait_vsync(void)
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




#pragma once
#include <windows.h>

extern int video_encoder_init(void);
extern void video_encoder_term(void);
extern HRESULT video_encoder_write_frame(BYTE* videoFrameBuffer);
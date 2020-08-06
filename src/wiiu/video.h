#ifndef WIILINKU_VIDEO_H_
#define WIILINKU_VIDEO_H_
#include "utils.h"


extern bool video_init(void);
extern void video_term(void);
extern void video_render_clear(void);
extern void video_render_flip(void);
extern void video_render_text(int x, int y, const char* str);
extern void video_render_text_aligned(int x, int y, const char* str);
extern void video_render_text_fmt(int x, int y, const char* fmt, ...);
extern void video_render_text_aligned_fmt(int x, int y, const char* fmt, ...);
extern void video_log_write(const char* fmt, ...);
#endif /* WIILINKU_VIDEO_H_ */

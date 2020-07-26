#ifndef WIIUPCX_VIDEO_H_
#define WIIUPCX_VIDEO_H_
#include "utils.h"


extern bool video_init(void);
extern void video_term(void);
extern void video_render_clear(void);
extern void video_render_flip(void);
extern void video_render_text(int x, int y, const char* buf);
extern void video_render_text_aligned(int x, int y, char* buf);



#endif /* WIIUPCX_VIDEO_H_ */

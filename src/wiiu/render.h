#ifndef WLU_RENDER_H_
#define WLU_RENDER_H_


bool render_init(void);
void render_term(void);
void render_write_texture(void* data);
void render_flush_n_wait_vsync(void);

#endif

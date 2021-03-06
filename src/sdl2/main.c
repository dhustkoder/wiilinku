#include <arpa/inet.h>
#include <sys/socket.h>
#include <ifaddrs.h>

#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_net.h>
#include "zui.h"


static SDL_Window* window;
static SDL_Renderer* renderer;
static SDL_Texture* texture;


bool platform_init(void)
{
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		log_error("Couldn't initialize SDL: %s\n", SDL_GetError());
		return false;
	}

	if (SDLNet_Init() != 0) {
		log_error("Couldn't initialize SDL_net: %s\n", SDL_GetError());
		return false;
	}

	window = SDL_CreateWindow(
		"WiiLinkU " WLU_VERSION_STR, 
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		ZUI_WIDTH,
		ZUI_HEIGHT,
		SDL_WINDOW_SHOWN|SDL_WINDOW_RESIZABLE
	);
	if (window == NULL) {
		log_error("Failed to create SDL_Window: %s\n", SDL_GetError());
		goto Lquitsdl;
	}

	renderer = SDL_CreateRenderer(window, -1, 0);
	if (renderer == NULL) {
		log_error("Failed to create SDL_Renderer: %s\n", SDL_GetError());
		goto Lfreewindow;
	}

	texture = SDL_CreateTexture(
		renderer,
		SDL_PIXELFORMAT_RGB24,
		SDL_TEXTUREACCESS_STREAMING,
		ZUI_WIDTH, ZUI_HEIGHT
	);
	if (texture == NULL) {
		log_error("Failed to create SDL_Texture: %s\n", SDL_GetError());
		goto Lfreerenderer;
	}

	SDL_RenderClear(renderer);
	SDL_RenderPresent(renderer);
	return true;


	SDL_DestroyTexture(texture);
Lfreerenderer:
	SDL_DestroyRenderer(renderer);
Lfreewindow:
	SDL_DestroyWindow(window);
Lquitsdl:
	SDL_Quit();
	return false;
}

static void platform_term(void)
{
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}


static bool update_events(void)
{
	SDL_Event event;
	while (SDL_PollEvent(&event) != 0) {
		switch (event.type) {
		case SDL_QUIT:
			log_info("SDL QUIT");
			return false;
		}
	}

	return true;
}

int main(void)
{
	if (!platform_init()) {
		return -1;
	}

	log_info("MAX_ALIGNMENT_SIZE: %lu", MAX_ALIGNMENT_SIZE);

    struct ifaddrs *ifap, *ifa;
    struct sockaddr_in *sa;
    char *addr;

    getifaddrs (&ifap);
    for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr && ifa->ifa_addr->sa_family==AF_INET) {
            sa = (struct sockaddr_in *) ifa->ifa_addr;
            addr = inet_ntoa(sa->sin_addr);
            if (strcmp(addr, "127.0.0.1") != 0 && strcmp(addr, "0.0.0.0") != 0) {
            	log_info("Interface: %s\tAddress: %s\n", ifa->ifa_name, addr);
            	break;
            }
        }
    }

    freeifaddrs(ifap);

	zui_obj_id_t zui_tcnt_strs[5];
	char tbuf[64];
	unsigned int tcnt_vals[5] = {  0, 0, 0, 0, 0 };
	int tcnt_inc[5] =  { 1024, 1024 * 2, 1024 * 4, 1024 * 8, 1024 * 16 };
	zui_init();
	zui_obj_id_t text_ip_id    = zui_text_create(addr, (struct vec2i){24, 44});
	for (int i = 0; i < 5; ++i) {
		sprintf(tbuf, "%u", tcnt_vals[i]);
		zui_tcnt_strs[i] = zui_text_create(tbuf, (struct vec2i){ 320, 16 + i * 10});
	}
	zui_obj_id_t text_hello_id = zui_text_create("WiiLinkU: Hello SDL2", (struct vec2i){24, 24});

	while (update_events()) {

		for (int i = 0; i < 5; ++i) {
			tcnt_vals[i] += tcnt_inc[i];
			sprintf(tbuf, "%u", tcnt_vals[i]);
			zui_text_set(zui_tcnt_strs[i], tbuf);
		}

		if (zui_update()) {
			int pitch;
			void* pixels;
			SDL_LockTexture(texture, NULL, &pixels, &pitch);
			zui_render(pixels);
			SDL_UnlockTexture(texture);
			SDL_RenderClear(renderer);
			SDL_RenderCopy(renderer, texture, NULL, NULL);
		}

		SDL_RenderPresent(renderer);
	}

	zui_term();
	platform_term();
	return 0;
}


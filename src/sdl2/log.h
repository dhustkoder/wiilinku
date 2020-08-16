#include <SDL2/SDL.h>


#define log_info(...)  SDL_Log(__VA_ARGS__)
#define log_error(...) SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, __VA_ARGS__)
#ifdef WLU_DEBUG
#define log_debug(...) SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, __VA_ARGS__)
#else
#define log_debug(...) ((void)0)



#endif

all:
	mkdir -p macos_debug_build
	gcc -O0 -g -DDEBUG -DWIILINKU_DEBUG src/*.c src/sdl2/*.c -Isrc/ -Isrc/sdl2 $(shell sdl2-config --cflags) -o macos_debug_build/wiilinku.app  $(shell sdl2-config --libs) -lSDL2_net
CC = g++
CFLAGS = -Wall -Wextra -std=c++17 -DWLR_USE_UNSTABLE `pkg-config --cflags sdl2 wayland-server wlroots`
LDFLAGS = `pkg-config --libs sdl2` -lwayland-server

# Use gcc for wlroots-related C compat
C_COMPILER = gcc
CFLAGS_C = -Wall -Wextra -std=c99 -DWLR_USE_UNSTABLE `pkg-config --cflags wayland-server wlroots`
LDFLAGS_C = `pkg-config --libs wlroots` -lwayland-server

all: compositor

src/wlroots_compositor.o: src/wlroots_compositor.c
	$(C_COMPILER) $(CFLAGS_C) -c src/wlroots_compositor.c -o src/wlroots_compositor.o

compositor: src/main.cpp src/wlroots_compositor.o
	$(CC) $(CFLAGS) src/main.cpp src/wlroots_compositor.o -o compositor $(LDFLAGS) $(LDFLAGS_C)

clean:
	rm -f compositor src/wlroots_compositor.o
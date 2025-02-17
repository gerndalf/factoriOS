#include <stdio.h>
#include <wayland-server.h>
#include <wlr/backend.h>
#include <wlr/backend.h>
#include <wlr/types/wlr_output.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/render/egl.h>
#include <wlr/render/pixman.h>
#include <wlr/util/log.h>

//global wayland struct
struct Server {
    struct wl_display *display;
    struct wl_event_loop *event_loop;
    struct wlr_backend *backend;
    struct wlr_renderer *renderer;
};

int start_compositor() {
    wlr_log_init(WLR_INFO, NULL); //init wlroots logging
    
    //wayland display
    struct Server server = {};
    server.display = wl_display_create();

    if (!server.display) {
        fprintf(stderr, "Failed to create Wayland display!\n");
        return 1;
    }

    // event loop
    server.event_loop = wl_display_get_event_loop(server.display);

    // backend
    server.backend = wlr_backend_autocreate(server.display, NULL);
    if (!server.backend) {
        fprintf(stderr, "Failed to create backend!\n");
        wl_display_destroy(server.display);
        return 1;
    }

    // renderer
    server.renderer = wlr_pixman_renderer_create();
    if (!server.renderer) {
        fprintf(stderr, "Failed to create renderer!\n");
        wlr_backend_destroy(server.backend);
        wl_display_destroy(server.display);
        return 1;
    }

    if (!wlr_backend_start(server.backend)) {
        fprintf(stderr, "Failed to start the backend!\n");
        wlr_renderer_destroy(server.renderer);
        wlr_backend_destroy(server.backend);
        wl_display_destroy(server.display);
        return 1;
    }

    printf("Wayland compositor running...\n");

    //run event loop
    wl_display_run(server.display);


    // Cleanup
    wl_display_destroy(server.display);
    wlr_renderer_destroy(server.renderer);
    wlr_backend_destroy(server.backend);

    return 0;
}
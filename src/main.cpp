#include <SDL2/SDL.h>
#include <iostream>
#include <thread>
#include <chrono>

extern "C" int start_compositor();

//window dimensions
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

//player props
SDL_Rect player = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 40, 40};
int playerSpeed = 5;

void handleInput(bool &running, const Uint8 *keyState) {
    if (keyState[SDL_SCANCODE_ESCAPE]) {
        running = false;
    }
    if (keyState[SDL_SCANCODE_W]) player.y -= playerSpeed;
    if (keyState[SDL_SCANCODE_S]) player.y += playerSpeed;
    if (keyState[SDL_SCANCODE_A]) player.x -= playerSpeed;
    if (keyState[SDL_SCANCODE_D]) player.x += playerSpeed;
}

int main() {
    std::thread compositorThread([](){
        start_compositor();
    });

    std::this_thread::sleep_for(std::chrono::seconds(1));
    

    // init sdl
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not init! SDL_Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    // create sdl2 window w/ wayland be
    SDL_Window *window = SDL_CreateWindow(
        "factoriOS", 
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
        SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN
    );

    if (!window) {
        std::cerr << "Window could not be create! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }


    //renderer
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if(!renderer) {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // game loop
    bool running = true;
    SDL_Event event;
    while(running) {
        SDL_PumpEvents();
        const Uint8 *keyState = SDL_GetKeyboardState(NULL);
        handleInput(running, keyState);

        //clear screen
        SDL_SetRenderDrawColor(renderer, 0,0,0,255);
        SDL_RenderClear(renderer);

        //draw player
        SDL_SetRenderDrawColor(renderer, 0,255,0,255); //GREEN PLAYER
        SDL_RenderFillRect(renderer, &player);

        //render
        SDL_RenderPresent(renderer);

        //frame cap
        SDL_Delay(16);
    }

    // Cleanup
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    compositorThread.detach();
    return 0;
}


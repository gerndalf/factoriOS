#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <filesystem>
#include <sys/inotify.h>
#include <unistd.h>


namespace fs = std::filesystem;

extern "C" int start_compositor();

//window dimensions
const int SCREEN_WIDTH = 1200;
const int SCREEN_HEIGHT = 800;
const int TILE_SIZE = 50;
const int TILE_PADDING = 50;

SDL_Window *window = nullptr;
SDL_Renderer *renderer = nullptr;
TTF_Font *font = nullptr;

//player props
SDL_Rect player = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 40, 40};
int playerSpeed = 5;

struct FolderTile {
    SDL_Rect rect;
    std::string name;
};

std::vector<FolderTile> folderTiles;

//Scan ~/ and gen tiles
void loadFolders() {
    folderTiles.clear();
    std::string homePath = getenv("HOME");

    int x = TILE_PADDING, y = 100;
    for (const auto &entry : fs::directory_iterator(homePath)) {
        if (entry.is_directory() && entry.path().filename().string()[0] != '.') {
            folderTiles.push_back({{x, y, TILE_SIZE, TILE_SIZE}, entry.path().filename().string()});
            x += TILE_SIZE + TILE_PADDING;
            if (x + TILE_SIZE > SCREEN_WIDTH) {
                x = TILE_PADDING;
                y += TILE_SIZE + TILE_PADDING;
            }
        }
    }
}

void renderText(const std::string &text, int x, int y) {
    SDL_Color color = {255,255,255,255};
    SDL_Surface *surface = TTF_RenderText_Solid(font, text.c_str(), color);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect textRect = {x, y, surface->w, surface->h};

    SDL_RenderCopy(renderer, texture, nullptr, &textRect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

void monitorFolders(bool &running) {
    int inotifyFd = inotify_init();
    if (inotifyFd < 0) {
        std::cerr << "Failed to initialize inotify\n";
        return;
    }

    std::string homePath = getenv("HOME");
    int wd = inotify_add_watch(inotifyFd, homePath.c_str(), IN_CREATE | IN_DELETE);

    if (wd < 0 ) {
        std::cerr << "Failed to watch director\n";
        close(inotifyFd);
        return;
    }

    char buffer[1024];
    while (running) {
        int length = read(inotifyFd, buffer, sizeof(buffer));
        if (length > 0) {
            loadFolders();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    inotify_rm_watch(inotifyFd, wd);
    close(inotifyFd);
}

void handleInput(bool &running, const Uint8 *keyState) {
    if (keyState[SDL_SCANCODE_ESCAPE]) {
        running = false;
    }
    if (keyState[SDL_SCANCODE_W]) player.y -= playerSpeed;
    if (keyState[SDL_SCANCODE_S]) player.y += playerSpeed;
    if (keyState[SDL_SCANCODE_A]) player.x -= playerSpeed;
    if (keyState[SDL_SCANCODE_D]) player.x += playerSpeed;
}

bool initSDL() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL failed to init " << SDL_GetError() << std::endl;
        return false;
    }
    if (TTF_Init() == -1) {
        std::cerr << "SDL_ttf failed to init " << TTF_GetError() << std::endl;
        return false;
    }
    window = SDL_CreateWindow("factoriOS", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Window failed to create " << SDL_GetError() << std::endl;
        return false; 
    }
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Renderer failed to create " << SDL_GetError() << std::endl;
        return false;
    }
    font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 16);
    if (!font) {
        std::cerr << "Font failed to create " << TTF_GetError() << std::endl;
        return false;
    }

    return true;
}

void closeSDL() {
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
}

int main() {
    std::thread compositorThread([](){
        start_compositor();
    });

    std::this_thread::sleep_for(std::chrono::seconds(1));
    

    // init sdl
    if (!initSDL()) {
        return 1;
    }

    loadFolders();

    // game loop
    bool running = true;
    std::thread monitorThread(monitorFolders, std::ref(running));

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

        //draw file tiles
        for (const auto &folder : folderTiles) {
            SDL_SetRenderDrawColor(renderer, 0,0,255,255);
            SDL_RenderFillRect(renderer, &folder.rect);
            renderText(folder.name, folder.rect.x, folder.rect.y - 20);
        }


        //render
        SDL_RenderPresent(renderer);

        //frame cap
        SDL_Delay(16);
    }

    // Cleanup
    running = false;
    monitorThread.detach();
    compositorThread.detach();
    
    closeSDL();
    return 0;
}


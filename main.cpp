#include <SDL.h>
#include <SDL_image.h>
#include <iostream>

const int SCREEN_WIDTH = 480;
const int SCREEN_HEIGHT = 800;
const int PLAYER_WIDTH = 50;
const int PLAYER_HEIGHT = 50;

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
SDL_Texture* backgroundTexture = nullptr;
SDL_Texture* ninjaTexture = nullptr;
SDL_Texture* wallTexture = nullptr;

SDL_Texture* loadTexture(const char* path) {
    SDL_Surface* loadedSurface = IMG_Load(path);
    if (!loadedSurface) {
        std::cout << "IMG_Load failed: " << IMG_GetError() << "\n";
        return nullptr;
    }
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, loadedSurface);
    SDL_FreeSurface(loadedSurface);
    return tex;
}

struct Player {
    int x, y;
    bool onLeftWall;

    Player() {
        x = 50;
        y = SCREEN_HEIGHT - 100;
        onLeftWall = true;
    }

    void jump() {
        onLeftWall = !onLeftWall;
        x = onLeftWall ? 50 : SCREEN_WIDTH - 50 - PLAYER_WIDTH;
    }

    void update() {
        y -= 2;
        if (y < -PLAYER_HEIGHT) y = SCREEN_HEIGHT;
    }

    void draw() {
        SDL_Rect rect = { x, y, PLAYER_WIDTH, PLAYER_HEIGHT };
        SDL_RendererFlip flip = onLeftWall ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;
        SDL_RenderCopyEx(renderer, ninjaTexture, nullptr, &rect, 0, nullptr, flip);
    }
};

bool init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "SDL init failed: " << SDL_GetError() << "\n";
        return false;
    }

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        std::cout << "SDL_image init failed: " << IMG_GetError() << "\n";
        return false;
    }

    window = SDL_CreateWindow("Cute NinJump", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) return false;

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    return renderer != nullptr;
}

void close() {
    SDL_DestroyTexture(backgroundTexture);
    SDL_DestroyTexture(ninjaTexture);
    SDL_DestroyTexture(wallTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
}

int main(int argc, char* args[]) {
    if (!init()) return -1;

    wallTexture = loadTexture("wall.jpg");
    backgroundTexture = loadTexture("background.jpg");
    ninjaTexture = loadTexture("ninja3.png");

    if (!backgroundTexture || !ninjaTexture || !wallTexture) {
        close();
        return -1;
    }

    int wallTextureWidth, wallTextureHeight;
    SDL_QueryTexture(wallTexture, nullptr, nullptr, &wallTextureWidth, &wallTextureHeight);

    bool quit = false;
    SDL_Event e;
    Player player;

    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT)
                quit = true;
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_SPACE)
                player.jump();
        }

        player.update();

        SDL_RenderClear(renderer);

        SDL_Rect leftWallRect = { 0, 0, 50, SCREEN_HEIGHT };
        SDL_Rect leftWallSrcRect = { 0, 0, 50, wallTextureHeight };
        SDL_RenderCopy(renderer, wallTexture, &leftWallSrcRect, &leftWallRect);

        SDL_Rect rightWallRect = { SCREEN_WIDTH - 50, 0, 50, SCREEN_HEIGHT };
        SDL_Rect rightWallSrcRect = { wallTextureWidth - 50, 0, 50, wallTextureHeight };
        SDL_RenderCopy(renderer, wallTexture, &rightWallSrcRect, &rightWallRect);

        SDL_Rect backgroundRect = { 50, 0, SCREEN_WIDTH - 100, SCREEN_HEIGHT };
        SDL_RenderCopy(renderer, backgroundTexture, nullptr, &backgroundRect);

        player.draw();

        SDL_RenderPresent(renderer);
        SDL_Delay(12);
    }

    close();
    return 0;
}

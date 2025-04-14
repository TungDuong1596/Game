#include <SDL.h>
#include <iostream>

const int SCREEN_WIDTH = 480;
const int SCREEN_HEIGHT = 800;
const int PLAYER_WIDTH = 40;
const int PLAYER_HEIGHT = 40;
const int WALL_WIDTH = 50;

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;

struct Player {
    int x, y;
    bool onLeftWall;

    Player() {
        x = WALL_WIDTH;
        y = SCREEN_HEIGHT - 100;
        onLeftWall = true;
    }

    void jump() {
        onLeftWall = !onLeftWall;
        x = onLeftWall ? WALL_WIDTH : SCREEN_WIDTH - WALL_WIDTH - PLAYER_WIDTH;
    }

    void update() {
        y -= 2; // Tự động leo lên
        if (y < -PLAYER_HEIGHT) y = SCREEN_HEIGHT; // reset về dưới nếu quá đỉnh
    }

    void draw() {
        SDL_Rect rect = { x, y, PLAYER_WIDTH, PLAYER_HEIGHT };
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // đỏ
        SDL_RenderFillRect(renderer, &rect);
    }
};

bool init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "SDL init failed: " << SDL_GetError() << "\n";
        return false;
    }

    window = SDL_CreateWindow("NinJump SDL2", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) return false;

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    return renderer != nullptr;
}

void close() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

int main(int argc, char* args[]) {
    if (!init()) return -1;

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

        // Clear screen
        SDL_SetRenderDrawColor(renderer, 200, 200, 255, 255); // màu nền
        SDL_RenderClear(renderer);

        // Draw walls
        SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
        SDL_Rect leftWall = { 0, 0, WALL_WIDTH, SCREEN_HEIGHT };
        SDL_Rect rightWall = { SCREEN_WIDTH - WALL_WIDTH, 0, WALL_WIDTH, SCREEN_HEIGHT };
        SDL_RenderFillRect(renderer, &leftWall);
        SDL_RenderFillRect(renderer, &rightWall);

        // Draw player
        player.draw();

        // Update screen
        SDL_RenderPresent(renderer);
        SDL_Delay(16); // ~60fps
    }

    close();
    return 0;
}

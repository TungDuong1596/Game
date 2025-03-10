#include "Engine.h"
#include <SDL.h>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <algorithm>

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int PLAYER_SIZE = 40;
const int PLAYER_SPEED = 5;

struct Entity {
    int x, y;
};

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Move Player", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    Entity player = {SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2};

    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
        }

// Di chuyển nhân vật người chơi với giới hạn biên
const Uint8* keys = SDL_GetKeyboardState(NULL);
if (keys[SDL_SCANCODE_W] && player.y - PLAYER_SPEED > PLAYER_SIZE)
    player.y -= PLAYER_SPEED;
if (keys[SDL_SCANCODE_S] && player.y + PLAYER_SPEED < SCREEN_HEIGHT - PLAYER_SIZE)
    player.y += PLAYER_SPEED;
if (keys[SDL_SCANCODE_A] && player.x - PLAYER_SPEED > PLAYER_SIZE)
    player.x -= PLAYER_SPEED;
if (keys[SDL_SCANCODE_D] && player.x + PLAYER_SPEED < SCREEN_WIDTH - PLAYER_SIZE)
    player.x += PLAYER_SPEED;



        // Vẽ màn hình
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Vẽ người chơi
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_Rect playerRect = {player.x - PLAYER_SIZE / 2, player.y - PLAYER_SIZE / 2, PLAYER_SIZE, PLAYER_SIZE};
        SDL_RenderFillRect(renderer, &playerRect);

        SDL_RenderPresent(renderer);
        SDL_Delay(1000 / 60);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}


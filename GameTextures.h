#pragma once
#include <SDL.h>

struct GameTextures {
    SDL_Texture* background = nullptr;
    SDL_Texture* ninja = nullptr;
    SDL_Texture* wall = nullptr;
    SDL_Texture* platform = nullptr;
    SDL_Texture* heart = nullptr;
    SDL_Texture* menu = nullptr;
    SDL_Texture* gameOver = nullptr;
    SDL_Texture* pause = nullptr;
};

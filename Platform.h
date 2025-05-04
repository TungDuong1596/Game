#pragma once
#include <SDL.h>
#include "constants.h"

using namespace std;

struct Platform {
    SDL_Rect rect;
    float alpha;

    Platform(int x, int y);
    void update(float speed);
    void render(SDL_Renderer* renderer, SDL_Texture* texture);
};

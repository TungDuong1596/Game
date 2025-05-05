#pragma once
#include <SDL.h>
#include "constants.h"

class Shuriken {
public:
    Shuriken(int x, int y);
    void update();
    void render(SDL_Renderer* renderer, SDL_Texture* texture);
    SDL_Rect getRect() const;
    bool isActive() const;
    void deactivate() { active = false; }

private:
    SDL_Rect rect;
    bool active;
    static const int SHURIKEN_SPEED = 10;
    static const int SHURIKEN_WIDTH = PLAYER_WIDTH / 2;
    static const int SHURIKEN_HEIGHT = PLAYER_HEIGHT / 2;
};

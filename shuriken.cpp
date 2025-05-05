#include "shuriken.h"

Shuriken::Shuriken(int x, int y) : active(true) {
    rect = { x - SHURIKEN_WIDTH/2, y - SHURIKEN_HEIGHT, SHURIKEN_WIDTH, SHURIKEN_HEIGHT };
}

void Shuriken::update() {
    rect.y -= SHURIKEN_SPEED;

    if (rect.y < -SHURIKEN_HEIGHT) {
        active = false;
    }
}

void Shuriken::render(SDL_Renderer* renderer, SDL_Texture* texture) {
    if (active) {
        SDL_RenderCopy(renderer, texture, nullptr, &rect);
    }
}

SDL_Rect Shuriken::getRect() const { return rect; }
bool Shuriken::isActive() const { return active; }

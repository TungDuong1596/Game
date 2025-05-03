#include "Platform.h"
#include "constants.h"

Platform::Platform(int x, int y) : alpha(255.0f) {
    rect = { x, y, PLATFORM_WIDTH, PLATFORM_HEIGHT };
}

void Platform::update(float speed) {
    rect.y += static_cast<int>(speed);
    if (alpha > 0) alpha -= 1.5f;
}

void Platform::render(SDL_Renderer* renderer, SDL_Texture* texture) {
    SDL_SetTextureAlphaMod(texture, static_cast<Uint8>(alpha));
    SDL_RenderCopy(renderer, texture, nullptr, &rect);
}

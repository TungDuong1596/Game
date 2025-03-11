#include <SDL.h>
#include <SDL_ttf.h>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <algorithm>
#include <string>

const int SCREEN_WIDTH = 1200;
const int SCREEN_HEIGHT = 900;
const int PLAYER_SIZE = 40;
const int PLAYER_SPEED = 5;
const float GRAVITY = 0.5f;
const float JUMP_FORCE = -25.0f;
const int DASH_SPEED = 50;
const int DASH_DURATION = 10;
const int DASH_COOLDOWN = 1800; // 30 giây (60 FPS * 30)

struct Entity {
    float x, y;
    float velocity_y = 0.0f;
    bool isGrounded = false;
    bool isDashing = false;
    int dashTimer = 0;
    int dashDirection = 1;
    int dashCooldown = 0;
};

void renderText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, int x, int y) {
    SDL_Color white = {255, 255, 255, 255};
    SDL_Surface* surface = TTF_RenderText_Solid(font, text.c_str(), white);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect destRect = {x, y, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, NULL, &destRect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0 || TTF_Init() < 0) {
        SDL_Log("SDL could not initialize! SDL_Error: %s", SDL_GetError());
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow("Move Player with Gravity & Dash", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        SDL_Log("Window could not be created! SDL_Error: %s", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        SDL_Log("Renderer could not be created! SDL_Error: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    TTF_Font* font = TTF_OpenFont("arial.ttf", 24);
    if (!font) {
        SDL_Log("Failed to load font! SDL_ttf Error: %s", TTF_GetError());
    }

    Entity player = {SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f};

    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
        }

        const Uint8* keys = SDL_GetKeyboardState(NULL);
        if (!player.isDashing) {
            if (keys[SDL_SCANCODE_A] && player.x - PLAYER_SPEED >= 0)
                player.x -= PLAYER_SPEED;
            if (keys[SDL_SCANCODE_D] && player.x + PLAYER_SIZE + PLAYER_SPEED <= SCREEN_WIDTH)
                player.x += PLAYER_SPEED;
        }

        if (keys[SDL_SCANCODE_SPACE] && player.isGrounded) {
            player.velocity_y = JUMP_FORCE;
            player.isGrounded = false;
        }

        if (keys[SDL_SCANCODE_E] && !player.isDashing && player.dashCooldown == 0) {
            player.isDashing = true;
            player.dashTimer = DASH_DURATION;
            player.dashCooldown = DASH_COOLDOWN;
            player.dashDirection = (keys[SDL_SCANCODE_A]) ? -1 : (keys[SDL_SCANCODE_D] ? 1 : player.dashDirection);
        }

        if (player.isDashing) {
            player.x += DASH_SPEED * player.dashDirection;
            if (player.x < 0) player.x = 0;
            if (player.x + PLAYER_SIZE > SCREEN_WIDTH) player.x = SCREEN_WIDTH - PLAYER_SIZE;
            player.dashTimer--;
            if (player.dashTimer <= 0) {
                player.isDashing = false;
            }
        } else {
            player.velocity_y += GRAVITY;
            player.y += player.velocity_y;
        }

        if (player.dashCooldown > 0) {
            player.dashCooldown--;
        }

        if (player.y + PLAYER_SIZE >= SCREEN_HEIGHT) {
            player.y = SCREEN_HEIGHT - PLAYER_SIZE;
            player.velocity_y = 0;
            player.isGrounded = true;
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 216, 191, 216, 255);
        SDL_Rect playerRect = {(int)player.x, (int)player.y, PLAYER_SIZE, PLAYER_SIZE};
        SDL_RenderFillRect(renderer, &playerRect);

        if (font) {
            std::string cooldownText = "Dash Cooldown: " + std::to_string(player.dashCooldown / 60) + "s";
            renderText(renderer, font, cooldownText, 50, 50);
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(1000 / 60);
    }

    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 0;
}

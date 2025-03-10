#include <SDL.h>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <algorithm>

const int SCREEN_WIDTH = 1200;
const int SCREEN_HEIGHT = 900;
const int PLAYER_SIZE = 40;
const int PLAYER_SPEED = 5;
const float GRAVITY = 0.5f;  // Trọng lực
const float JUMP_FORCE = -15.0f; // Lực nhảy
a

struct Entity {
    float x, y;
    float velocity_y = 0.0f; // Vận tốc rơi
    bool isGrounded = false; // Kiểm tra nếu đang chạm đất
};

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_Log("SDL could not initialize! SDL_Error: %s", SDL_GetError());
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow("Move Player with Gravity", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
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

    Entity player = {SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f};

    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
        }

        // Nhận phím điều khiển
        const Uint8* keys = SDL_GetKeyboardState(NULL);
        if (keys[SDL_SCANCODE_A] && player.x - PLAYER_SPEED >= 0)
            player.x -= PLAYER_SPEED;
        if (keys[SDL_SCANCODE_D] && player.x + PLAYER_SIZE + PLAYER_SPEED <= SCREEN_WIDTH)
            player.x += PLAYER_SPEED;

        // Nhảy nếu đang đứng trên mặt đất
        if (keys[SDL_SCANCODE_SPACE] && player.isGrounded) {
            player.velocity_y = JUMP_FORCE; // Nhảy lên
            player.isGrounded = false;
        }

        // Áp dụng trọng lực
        player.velocity_y += GRAVITY;
        player.y += player.velocity_y;

        // Va chạm với mặt đất
        if (player.y + PLAYER_SIZE >= SCREEN_HEIGHT) {
            player.y = SCREEN_HEIGHT - PLAYER_SIZE; // Giữ thực thể trên mặt đất
            player.velocity_y = 0; // Dừng rơi
            player.isGrounded = true;
        }

        // Vẽ màn hình
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Vẽ người chơi
        SDL_SetRenderDrawColor(renderer, 216, 191, 216, 255);
        SDL_Rect playerRect = {(int)player.x, (int)player.y, PLAYER_SIZE, PLAYER_SIZE};
        SDL_RenderFillRect(renderer, &playerRect);

        SDL_RenderPresent(renderer);
        SDL_Delay(1000 / 60);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

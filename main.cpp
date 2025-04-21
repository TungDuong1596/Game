#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <iostream>
#include <vector>
#include <ctime>
#include <string>
#include <memory>
#include <algorithm>

const int SCREEN_WIDTH = 480;
const int SCREEN_HEIGHT = 800;
const int PLAYER_WIDTH = 28;
const int PLAYER_HEIGHT = 44;
const int PLATFORM_WIDTH = 25;
const int PLATFORM_HEIGHT = 25;
const int WALL_WIDTH = 50;
const int GRAVITY = 1;
const int JUMP_FORCE = -20;
const int HEART_SIZE = 32;
const int HEART_PADDING = 10;

enum class GameState { MENU, PLAYING, GAME_OVER, QUIT };

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
SDL_Texture* backgroundTexture = nullptr;
SDL_Texture* ninjaTexture = nullptr;
SDL_Texture* wallTexture = nullptr;
SDL_Texture* platformTexture = nullptr;
SDL_Texture* heartTexture = nullptr;
Mix_Chunk* jumpSound = nullptr;
Mix_Chunk* hitSound = nullptr;
Mix_Chunk* loseLifeSound = nullptr;
Mix_Chunk* gameOverSound = nullptr;

GameState gameState = GameState::MENU;

struct Platform {
    SDL_Rect rect;
    float alpha;
    Platform(int x, int y) {
        rect = { x, y, PLATFORM_WIDTH, PLATFORM_HEIGHT };
        alpha = 255.0f;
    }
    void update(float speed) {
        rect.y += static_cast<int>(speed);
        if (alpha > 0) alpha -= 1.5f;
    }
    void draw() {
        SDL_SetTextureAlphaMod(platformTexture, static_cast<Uint8>(alpha));
        SDL_RenderCopy(renderer, platformTexture, nullptr, &rect);
    }
};

struct Player {
    int x, y;
    float velocityY;
    bool onLeftWall;
    bool isJumping;
    bool isAttached;
    int score;
    float scoreMultiplier;
    Uint32 lastMultiplierIncreaseTime;
    int lives;
    bool isInvincible;
    Uint32 invincibleTime;
    int targetY;
    Uint32 lastScoreUpdateTime;

    Player()
        : x(WALL_WIDTH), y(SCREEN_HEIGHT - 100 - PLAYER_HEIGHT), velocityY(0), onLeftWall(true),
        isJumping(false), isAttached(true), score(0), scoreMultiplier(1.0f),
        lastMultiplierIncreaseTime(SDL_GetTicks()), lives(3), isInvincible(false),
        invincibleTime(0), targetY(SCREEN_HEIGHT - 100 - PLAYER_HEIGHT),
        lastScoreUpdateTime(SDL_GetTicks()) {}

    void jump() {
        if (isAttached) {
            isAttached = false;
            onLeftWall = !onLeftWall;
            velocityY = JUMP_FORCE;
            isJumping = true;
            targetY = y;
            Mix_PlayChannel(-1, jumpSound, 0);
        }
    }

    void update() {
        if (!isAttached) {
            velocityY += GRAVITY;
            y += velocityY;
            if ((velocityY > 0 && y >= targetY) || (velocityY < 0 && y <= targetY)) {
                y = targetY;
                isAttached = true;
                velocityY = 0;
                isJumping = false;
                x = onLeftWall ? WALL_WIDTH : SCREEN_WIDTH - WALL_WIDTH - PLAYER_WIDTH;
            }
        }

        if (y > SCREEN_HEIGHT) {
            reset();
        }

        Uint32 currentTime = SDL_GetTicks();
        if (currentTime - lastMultiplierIncreaseTime > 10000) {
            scoreMultiplier += 0.5f;
            lastMultiplierIncreaseTime = currentTime;
        }

        if (isInvincible && currentTime > invincibleTime) {
            isInvincible = false;
        }

        if (currentTime - lastScoreUpdateTime >= 1000) {
            score += static_cast<int>(scoreMultiplier);
            lastScoreUpdateTime = currentTime;
        }
    }

    void reset() {
        y = SCREEN_HEIGHT - 100 - PLAYER_HEIGHT;
        targetY = y;
        x = WALL_WIDTH;
        velocityY = 0;
        onLeftWall = true;
        isJumping = false;
        isAttached = true;
        score = 0;
        scoreMultiplier = 1.0f;
        lastMultiplierIncreaseTime = SDL_GetTicks();
        lives = 3;
        isInvincible = false;
        lastScoreUpdateTime = SDL_GetTicks();
    }

    void resetPosition() {
        y = SCREEN_HEIGHT - 100 - PLAYER_HEIGHT;
        targetY = y;
        x = onLeftWall ? WALL_WIDTH : SCREEN_WIDTH - WALL_WIDTH - PLAYER_WIDTH;
        velocityY = 0;
        isAttached = true;
        isJumping = false;
        isInvincible = true;
        invincibleTime = SDL_GetTicks() + 2000;
    }

    void draw() {
        SDL_Rect destRect = { x, y, PLAYER_WIDTH, PLAYER_HEIGHT };
        SDL_RendererFlip flip = onLeftWall ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;
        SDL_RenderCopyEx(renderer, ninjaTexture, nullptr, &destRect, 0, nullptr, flip);
    }

    SDL_Rect getRect() const {
        return { x, y, PLAYER_WIDTH, PLAYER_HEIGHT };
    }
};

SDL_Texture* loadTexture(const char* path) {
    SDL_Surface* surf = IMG_Load(path);
    if (!surf) return nullptr;
    SDL_Surface* converted = SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_RGBA8888, 0);
    SDL_FreeSurface(surf);
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, converted);
    SDL_FreeSurface(converted);
    return tex;
}

bool init() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) return false;
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) return false;
    if (TTF_Init() == -1) return false;
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) return false;
    window = SDL_CreateWindow("NinJump", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) return false;
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) return false;
    return true;
}

void close() {
    SDL_DestroyTexture(backgroundTexture);
    SDL_DestroyTexture(ninjaTexture);
    SDL_DestroyTexture(wallTexture);
    SDL_DestroyTexture(platformTexture);
    SDL_DestroyTexture(heartTexture);
    Mix_FreeChunk(jumpSound);
    Mix_FreeChunk(hitSound);
    Mix_FreeChunk(loseLifeSound);
    Mix_FreeChunk(gameOverSound);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    Mix_Quit();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

bool checkCollision(const SDL_Rect& a, const SDL_Rect& b) {
    return (a.x < b.x + b.w && a.x + a.w > b.x && a.y < b.y + b.h && a.y + a.h > b.y);
}

void renderTextCentered(const std::string& text, TTF_Font* font, SDL_Color color, int yOffset) {
    SDL_Surface* surface = TTF_RenderText_Solid(font, text.c_str(), color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect rect = { SCREEN_WIDTH / 2 - surface->w / 2, SCREEN_HEIGHT / 2 + yOffset, surface->w, surface->h };
    SDL_FreeSurface(surface);
    SDL_RenderCopy(renderer, texture, nullptr, &rect);
    SDL_DestroyTexture(texture);
}

int main(int argc, char* args[]) {
    if (!init()) return -1;

    wallTexture = loadTexture("wall.png");
    backgroundTexture = loadTexture("background.png");
    ninjaTexture = loadTexture("ninja.png");
    platformTexture = loadTexture("platform.png");
    heartTexture = loadTexture("heart.gif");
    jumpSound = Mix_LoadWAV("jump.wav");
    hitSound = Mix_LoadWAV("hit.wav");
    loseLifeSound = Mix_LoadWAV("lose_life.wav");
    gameOverSound = Mix_LoadWAV("game_over.wav");

    if (!wallTexture || !backgroundTexture || !ninjaTexture || !platformTexture || !heartTexture || !jumpSound || !hitSound || !loseLifeSound || !gameOverSound) {
        close(); return -1;
    }

    TTF_Font* font = TTF_OpenFont("arial.ttf", 24);
    if (!font) return -1;

    Player player;
    std::vector<Platform> platforms;
    srand((unsigned)time(nullptr));

    platforms.emplace_back(WALL_WIDTH, player.y - SCREEN_HEIGHT);

    SDL_Event e;
    Uint32 lastTime = SDL_GetTicks();
    const Uint32 frameDelay = 16;
    float platformSpeed = 3.0f;
    float speedIncreaseRate = 0.001f;
    float maxSpeed = 10.0f;

    while (gameState != GameState::QUIT) {
        Uint32 currentTime = SDL_GetTicks();
        Uint32 deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) gameState = GameState::QUIT;
            if (gameState == GameState::MENU && e.type == SDL_MOUSEBUTTONDOWN) gameState = GameState::PLAYING;
            if (gameState == GameState::PLAYING && e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_SPACE) player.jump();
            if (gameState == GameState::GAME_OVER && e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_r) {
                    player.reset();
                    platforms.clear();
                    platforms.emplace_back(WALL_WIDTH, player.y - SCREEN_HEIGHT);
                    gameState = GameState::PLAYING;
                } else if (e.key.keysym.sym == SDLK_ESCAPE) {
                    gameState = GameState::QUIT;
                }
            }
        }

        if (gameState == GameState::MENU) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);
            renderTextCentered("CLICK TO START", font, { 255, 255, 255 }, 0);
            SDL_RenderPresent(renderer);
            continue;
        }

        if (gameState == GameState::PLAYING) {
            player.update();

            for (auto& p : platforms) {
                p.update(platformSpeed);
            }

            for (auto& p : platforms) {
                if (!player.isInvincible && checkCollision(player.getRect(), p.rect)) {
                    Mix_PlayChannel(-1, hitSound, 0);
                    player.lives--;
                    if (player.lives > 0) {
                        Mix_PlayChannel(-1, loseLifeSound, 0);
                        player.resetPosition();
                    } else {
                        Mix_PlayChannel(-1, gameOverSound, 0);
                        gameState = GameState::GAME_OVER;
                    }
                    break;
                }
            }

            if (platforms.empty() || platforms.back().rect.y > rand() % 40 + 80) {
                bool leftSide = (rand() % 2 == 0);
                int x = leftSide ? WALL_WIDTH : SCREEN_WIDTH - WALL_WIDTH - PLATFORM_WIDTH;
                int y = platforms.back().rect.y - (rand() % 50 + 130);
                platforms.emplace_back(x, y);
            }

            if (!platforms.empty() && platforms.front().rect.y > SCREEN_HEIGHT) {
                platforms.erase(platforms.begin());
            }

            if (platformSpeed < maxSpeed) platformSpeed += speedIncreaseRate;
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_Rect bgRect = { WALL_WIDTH, 0, SCREEN_WIDTH - 2 * WALL_WIDTH, SCREEN_HEIGHT };
        SDL_RenderCopy(renderer, backgroundTexture, nullptr, &bgRect);
        SDL_Rect leftWall = { 0, 0, WALL_WIDTH, SCREEN_HEIGHT };
        SDL_Rect rightWall = { SCREEN_WIDTH - WALL_WIDTH, 0, WALL_WIDTH, SCREEN_HEIGHT };
        SDL_RenderCopy(renderer, wallTexture, nullptr, &leftWall);
        SDL_RenderCopy(renderer, wallTexture, nullptr, &rightWall);

        for (auto& p : platforms) p.draw();
        player.draw();

        for (int i = 0; i < player.lives; ++i) {
            SDL_Rect heartRect = { 10 + i * (HEART_SIZE + HEART_PADDING), 10, HEART_SIZE, HEART_SIZE };
            SDL_RenderCopy(renderer, heartTexture, nullptr, &heartRect);
        }

        SDL_Color white = { 255, 255, 255 };
        renderTextCentered("Score: " + std::to_string(player.score), font, white, -SCREEN_HEIGHT / 2 + 30);

        if (gameState == GameState::GAME_OVER) {
            renderTextCentered("GAME OVER", font, { 255, 0, 0 }, -30);
            renderTextCentered("RESTART? (R) / NO (ESC)", font, white, 10);
        }

        SDL_RenderPresent(renderer);
        Uint32 frameTime = SDL_GetTicks() - currentTime;
        if (frameDelay > frameTime) SDL_Delay(frameDelay - frameTime);
    }

    TTF_CloseFont(font);
    close();
    return 0;
}

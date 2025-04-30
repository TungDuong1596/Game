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
#include <fstream>

namespace {
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
    const int INITIAL_PLATFORM_SPEED = 3;
    const int MAX_PLATFORM_SPEED = 10;
    const float SPEED_INCREASE_RATE = 0.001f;
    const int PLAYER_INVINCIBLE_TIME = 2000;
    const int MULTIPLIER_INCREASE_INTERVAL = 10000;
    const int SCORE_UPDATE_INTERVAL = 1000;
    const int PLATFORM_SPAWN_RANGE_MIN = 80;
    const int PLATFORM_SPAWN_RANGE_MAX = 130;
    const int PLATFORM_SPAWN_GAP_MIN = 40;
    const int PLATFORM_SPAWN_GAP_MAX = 80;
    const std::string HIGH_SCORE_FILE = "highscore.dat";
}

enum class GameState { MENU, PLAYING, PAUSED, GAME_OVER, QUIT };

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

struct GameSounds {
    Mix_Chunk* jump = nullptr;
    Mix_Chunk* hit = nullptr;
    Mix_Chunk* loseLife = nullptr;
    Mix_Chunk* gameOver = nullptr;
};

struct Platform {
    SDL_Rect rect;
    float alpha;

    Platform(int x, int y) : alpha(255.0f) {
        rect = { x, y, PLATFORM_WIDTH, PLATFORM_HEIGHT };
    }

    void update(float speed) {
        rect.y += static_cast<int>(speed);
        if (alpha > 0) alpha -= 1.5f;
    }

    void render(SDL_Renderer* renderer, SDL_Texture* texture) {
        SDL_SetTextureAlphaMod(texture, static_cast<Uint8>(alpha));
        SDL_RenderCopy(renderer, texture, nullptr, &rect);
    }
};

class Player {
public:
    int x, y;
    float velocityY;
    bool onLeftWall;
    bool isJumping;
    bool isAttached;
    int score;
    float scoreMultiplier;
    int lives;
    bool isInvincible;

    Player() : x(WALL_WIDTH), y(SCREEN_HEIGHT - 100 - PLAYER_HEIGHT), velocityY(0),
              onLeftWall(true), isJumping(false), isAttached(true), score(0),
              scoreMultiplier(1.0f), lives(3), isInvincible(false) {
        targetY = y;
        lastMultiplierIncreaseTime = SDL_GetTicks();
        lastScoreUpdateTime = SDL_GetTicks();
    }

    void jump(GameSounds& sounds) {
        if (isAttached) {
            isAttached = false;
            onLeftWall = !onLeftWall;
            velocityY = JUMP_FORCE;
            isJumping = true;
            targetY = y;
            Mix_PlayChannel(-1, sounds.jump, 0);
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
        if (currentTime - lastMultiplierIncreaseTime > MULTIPLIER_INCREASE_INTERVAL) {
            scoreMultiplier += 0.5f;
            lastMultiplierIncreaseTime = currentTime;
        }

        if (isInvincible && currentTime > invincibleTime) {
            isInvincible = false;
        }

        if (currentTime - lastScoreUpdateTime >= SCORE_UPDATE_INTERVAL) {
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
        invincibleTime = SDL_GetTicks() + PLAYER_INVINCIBLE_TIME;
    }

    void render(SDL_Renderer* renderer, SDL_Texture* texture) {
        SDL_Rect destRect = { x, y, PLAYER_WIDTH, PLAYER_HEIGHT };
        SDL_RendererFlip flip = onLeftWall ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;
        SDL_RenderCopyEx(renderer, texture, nullptr, &destRect, 0, nullptr, flip);
    }

    SDL_Rect getRect() const { return { x, y, PLAYER_WIDTH, PLAYER_HEIGHT }; }

private:
    int targetY;
    Uint32 lastMultiplierIncreaseTime;
    Uint32 invincibleTime;
    Uint32 lastScoreUpdateTime;
};

SDL_Texture* loadTexture(SDL_Renderer* renderer, const std::string& path) {
    SDL_Surface* surface = IMG_Load(path.c_str());
    if (!surface) {
        std::cerr << "Failed to load image: " << path << " - " << IMG_GetError() << std::endl;
        return nullptr;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    if (!texture) {
        std::cerr << "Failed to create texture: " << path << " - " << SDL_GetError() << std::endl;
    }

    return texture;
}

bool initSDL(SDL_Window*& window, SDL_Renderer*& renderer) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        std::cerr << "SDL initialization failed: " << SDL_GetError() << std::endl;
        return false;
    }

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        std::cerr << "SDL_image initialization failed: " << IMG_GetError() << std::endl;
        return false;
    }

    if (TTF_Init() == -1) {
        std::cerr << "SDL_ttf initialization failed: " << TTF_GetError() << std::endl;
        return false;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cerr << "SDL_mixer initialization failed: " << Mix_GetError() << std::endl;
        return false;
    }

    window = SDL_CreateWindow("NinJump", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                             SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        std::cerr << "Renderer creation failed: " << SDL_GetError() << std::endl;
        return false;
    }

    return true;
}

void cleanup(SDL_Window* window, SDL_Renderer* renderer, GameTextures& textures, GameSounds& sounds, TTF_Font* font) {
    SDL_DestroyTexture(textures.background);
    SDL_DestroyTexture(textures.ninja);
    SDL_DestroyTexture(textures.wall);
    SDL_DestroyTexture(textures.platform);
    SDL_DestroyTexture(textures.heart);
    SDL_DestroyTexture(textures.menu);
    SDL_DestroyTexture(textures.gameOver);
    SDL_DestroyTexture(textures.pause);

    Mix_FreeChunk(sounds.jump);
    Mix_FreeChunk(sounds.hit);
    Mix_FreeChunk(sounds.loseLife);
    Mix_FreeChunk(sounds.gameOver);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    TTF_CloseFont(font);

    Mix_Quit();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

bool loadResources(SDL_Renderer* renderer, GameTextures& textures, GameSounds& sounds) {
    textures.background = loadTexture(renderer, "background.png");
    textures.ninja = loadTexture(renderer, "ninja.png");
    textures.wall = loadTexture(renderer, "wall.png");
    textures.platform = loadTexture(renderer, "platform.png");
    textures.heart = loadTexture(renderer, "heart.gif");
    textures.menu = loadTexture(renderer, "menu.png");
    textures.gameOver = loadTexture(renderer, "background.png");
    textures.pause = loadTexture(renderer, "pause.png");

    sounds.jump = Mix_LoadWAV("jump.wav");
    sounds.hit = Mix_LoadWAV("hit.wav");
    sounds.loseLife = Mix_LoadWAV("lose_life.wav");
    sounds.gameOver = Mix_LoadWAV("game_over.wav");

    if (!textures.background || !textures.ninja || !textures.wall ||
        !textures.platform || !textures.heart || !textures.menu ||
        !textures.gameOver || !textures.pause ||
        !sounds.jump || !sounds.hit || !sounds.loseLife || !sounds.gameOver) {
        return false;
    }

    return true;
}

bool checkCollision(const SDL_Rect& a, const SDL_Rect& b) {
    return (a.x < b.x + b.w && a.x + a.w > b.x && a.y < b.y + b.h && a.y + a.h > b.y);
}

void renderText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text,
                SDL_Color color, int x, int y) {
    SDL_Surface* surface = TTF_RenderText_Solid(font, text.c_str(), color);
    if (!surface) return;

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (!texture) return;

    int texW = 0, texH = 0;
    SDL_QueryTexture(texture, nullptr, nullptr, &texW, &texH);
    SDL_Rect dstrect = { x, y, texW, texH };
    SDL_RenderCopy(renderer, texture, nullptr, &dstrect);
    SDL_DestroyTexture(texture);
}

void renderCenteredText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text,
                       SDL_Color color, int yOffset) {
    SDL_Surface* surface = TTF_RenderText_Solid(font, text.c_str(), color);
    if (!surface) return;

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (!texture) return;

    int texW, texH;
    SDL_QueryTexture(texture, nullptr, nullptr, &texW, &texH);
    SDL_Rect dstrect = { (SCREEN_WIDTH - texW)/2, SCREEN_HEIGHT/2 + yOffset, texW, texH };
    SDL_RenderCopy(renderer, texture, nullptr, &dstrect);
    SDL_DestroyTexture(texture);
}

void renderMenu(SDL_Renderer* renderer, SDL_Texture* menuTexture, TTF_Font* font, int highScore) {
    SDL_RenderCopy(renderer, menuTexture, nullptr, nullptr);

    if (highScore > 0) {
        renderCenteredText(renderer, font, "HIGH SCORE: " + std::to_string(highScore),
                         {255, 215, 0, 255}, -100);
    }
}

void renderPause(SDL_Renderer* renderer, SDL_Texture* pauseTexture) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
    SDL_Rect overlay = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    SDL_RenderFillRect(renderer, &overlay);
    SDL_RenderCopy(renderer, pauseTexture, nullptr, nullptr);
}

void renderGameOver(SDL_Renderer* renderer, SDL_Texture* gameOverTexture, TTF_Font* font,
                   int score, int highScore) {
    SDL_RenderCopy(renderer, gameOverTexture, nullptr, nullptr);

    renderCenteredText(renderer, font, "SCORE: " + std::to_string(score),
                      {0, 0, 0, 255}, -50);

    if (score == highScore) {
        renderCenteredText(renderer, font, "NEW HIGH SCORE!", {255, 215, 0, 255}, 0);
    }

    renderCenteredText(renderer, font, "M - MENU", {0, 0, 0, 255}, 50);
    renderCenteredText(renderer, font, "R - RESTART", {0, 0, 0, 255}, 100);
    renderCenteredText(renderer, font, "ESC - QUIT", {0, 0, 0, 255}, 150);
}

void renderHUD(SDL_Renderer* renderer, SDL_Texture* heartTexture, TTF_Font* font, int lives, int score) {
    for (int i = 0; i < lives; ++i) {
        SDL_Rect heartRect = { 10 + i * (HEART_SIZE + HEART_PADDING), 10, HEART_SIZE, HEART_SIZE };
        SDL_RenderCopy(renderer, heartTexture, nullptr, &heartRect);
    }
    renderText(renderer, font, "Score: " + std::to_string(score), {0, 0, 0, 255}, 10, 50);
}

void spawnPlatform(std::vector<Platform>& platforms) {
    if (platforms.empty() || platforms.back().rect.y > rand() % PLATFORM_SPAWN_GAP_MIN + PLATFORM_SPAWN_GAP_MAX) {
        bool leftSide = (rand() % 2 == 0);
        int x = leftSide ? WALL_WIDTH : SCREEN_WIDTH - WALL_WIDTH - PLATFORM_WIDTH;
        int y = platforms.empty() ? SCREEN_HEIGHT : platforms.back().rect.y - (rand() % PLATFORM_SPAWN_RANGE_MIN + PLATFORM_SPAWN_RANGE_MAX);
        platforms.emplace_back(x, y);
    }
}

int loadHighScore() {
    int highScore = 0;
    std::ifstream file(HIGH_SCORE_FILE, std::ios::binary);
    if (file) {
        file.read(reinterpret_cast<char*>(&highScore), sizeof(highScore));
        file.close();
    }
    return highScore;
}

void saveHighScore(int score) {
    std::ofstream file(HIGH_SCORE_FILE, std::ios::binary);
    if (file) {
        file.write(reinterpret_cast<const char*>(&score), sizeof(score));
        file.close();
    }
}

int main(int argc, char* args[]) {
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    GameTextures textures;
    GameSounds sounds;

    if (!initSDL(window, renderer)) {
        return -1;
    }

    TTF_Font* font = TTF_OpenFont("PixelifySans.ttf", 36);
    if (!font) {
        std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
        return -1;
    }

    if (!loadResources(renderer, textures, sounds)) {
        cleanup(window, renderer, textures, sounds, font);
        return -1;
    }

    Player player;
    std::vector<Platform> platforms;
    srand(static_cast<unsigned>(time(nullptr)));

    GameState gameState = GameState::MENU;
    float platformSpeed = INITIAL_PLATFORM_SPEED;
    int highScore = loadHighScore();

    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }

            switch (gameState) {
                case GameState::MENU:
                    if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_KEYDOWN) {
                        gameState = GameState::PLAYING;
                        player.reset();
                        platforms.clear();
                        platforms.emplace_back(WALL_WIDTH, player.y - SCREEN_HEIGHT);
                        platformSpeed = INITIAL_PLATFORM_SPEED;
                    }
                    break;

                case GameState::PLAYING:
                    if (event.type == SDL_KEYDOWN) {
                        if (event.key.keysym.sym == SDLK_SPACE) {
                            player.jump(sounds);
                        } else if (event.key.keysym.sym == SDLK_ESCAPE) {
                            gameState = GameState::PAUSED;
                        }
                    }
                    break;

                case GameState::PAUSED:
                    if (event.type == SDL_KEYDOWN) {
                        if (event.key.keysym.sym == SDLK_ESCAPE) {
                            gameState = GameState::PLAYING;
                        } else if (event.key.keysym.sym == SDLK_m) {
                            gameState = GameState::MENU;
                        }
                    }
                    break;

                case GameState::GAME_OVER:
                    if (event.type == SDL_KEYDOWN) {
                        if (event.key.keysym.sym == SDLK_r) {
                            gameState = GameState::PLAYING;
                            player.reset();
                            platforms.clear();
                            platforms.emplace_back(WALL_WIDTH, player.y - SCREEN_HEIGHT);
                            platformSpeed = INITIAL_PLATFORM_SPEED;
                        } else if (event.key.keysym.sym == SDLK_m) {
                            gameState = GameState::MENU;
                        } else if (event.key.keysym.sym == SDLK_ESCAPE) {
                            running = false;
                        }
                    }
                    break;

                default:
                    break;
            }
        }

        if (gameState == GameState::PLAYING) {
            player.update();

            for (auto& platform : platforms) {
                platform.update(platformSpeed);
            }

            for (auto& platform : platforms) {
                if (!player.isInvincible && checkCollision(player.getRect(), platform.rect)) {
                    Mix_PlayChannel(-1, sounds.hit, 0);
                    player.lives--;

                    if (player.lives > 0) {
                        Mix_PlayChannel(-1, sounds.loseLife, 0);
                        player.resetPosition();
                    } else {
                        Mix_PlayChannel(-1, sounds.gameOver, 0);
                        if (player.score > highScore) {
                            highScore = player.score;
                            saveHighScore(highScore);
                        }
                        gameState = GameState::GAME_OVER;
                    }
                    break;
                }
            }

            spawnPlatform(platforms);

            if (!platforms.empty() && platforms.front().rect.y > SCREEN_HEIGHT) {
                platforms.erase(platforms.begin());
            }

            if (platformSpeed < MAX_PLATFORM_SPEED) {
                platformSpeed += SPEED_INCREASE_RATE;
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        SDL_Rect bgRect = { WALL_WIDTH, 0, SCREEN_WIDTH - 2 * WALL_WIDTH, SCREEN_HEIGHT };
        SDL_RenderCopy(renderer, textures.background, nullptr, &bgRect);

        SDL_Rect leftWall = { 0, 0, WALL_WIDTH, SCREEN_HEIGHT };
        SDL_Rect rightWall = { SCREEN_WIDTH - WALL_WIDTH, 0, WALL_WIDTH, SCREEN_HEIGHT };
        SDL_RenderCopy(renderer, textures.wall, nullptr, &leftWall);
        SDL_RenderCopy(renderer, textures.wall, nullptr, &rightWall);

        for (auto& platform : platforms) {
            platform.render(renderer, textures.platform);
        }

        player.render(renderer, textures.ninja);

        if (gameState == GameState::PLAYING) {
            renderHUD(renderer, textures.heart, font, player.lives, player.score);
        }

        switch (gameState) {
            case GameState::MENU:
                renderMenu(renderer, textures.menu, font, highScore);
                break;

            case GameState::PAUSED:
                renderPause(renderer, textures.pause);
                break;

            case GameState::GAME_OVER:
                renderGameOver(renderer, textures.gameOver, font, player.score, highScore);
                break;

            default:
                break;
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    cleanup(window, renderer, textures, sounds, font);
    return 0;
}

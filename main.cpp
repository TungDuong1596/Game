#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <iostream>
#include <vector>
#include <ctime>
#include <string>

const int SCREEN_WIDTH = 480;
const int SCREEN_HEIGHT = 800;
const int PLAYER_WIDTH = 50;
const int PLAYER_HEIGHT = 50;
const int PLATFORM_WIDTH = 25;
const int PLATFORM_HEIGHT = 25;
const int WALL_WIDTH = 50;
const int GRAVITY = 1;
const int JUMP_FORCE = -20;

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
SDL_Texture* backgroundTexture = nullptr;
SDL_Texture* ninjaTexture = nullptr;
SDL_Texture* wallTexture = nullptr;
SDL_Texture* platformTexture = nullptr;

struct Platform {
    SDL_Rect rect;
    Platform(int x, int y) {
        rect = { x, y, PLATFORM_WIDTH, PLATFORM_HEIGHT };
    }
    void draw() {
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

    std::vector<SDL_Rect> animationFrames;
    int currentFrame;
    Uint32 frameTime;
    Uint32 animationSpeed;

    Player()
        : x(WALL_WIDTH),
        y(SCREEN_HEIGHT - 100),
        velocityY(0),
        onLeftWall(true),
        isJumping(false),
        isAttached(true),
        score(0),
        currentFrame(0),
        frameTime(0),
        animationSpeed(100) {
        initAnimation();
    }

    void initAnimation() {
        int totalFrames = 8;
        for (int i = 0; i < totalFrames; i++) {
            animationFrames.push_back({0, i * PLAYER_HEIGHT, PLAYER_WIDTH, PLAYER_HEIGHT});
        }
    }

    void updateAnimation() {
        Uint32 currentTime = SDL_GetTicks();
        if (currentTime - frameTime > animationSpeed) {
            frameTime = currentTime;
            currentFrame = (currentFrame + 1) % animationFrames.size();
        }
    }

    void jump() {
        if (isAttached) {
            isAttached = false;
            onLeftWall = !onLeftWall;
            velocityY = JUMP_FORCE;
            isJumping = true;
            animationSpeed = 50;
        }
    }

    void update() {
        if (!isAttached) {
            velocityY += GRAVITY;
            y += velocityY;

            if (onLeftWall) {
                if (x > WALL_WIDTH) {
                    x = WALL_WIDTH;
                    isAttached = true;
                    velocityY = 0;
                    isJumping = false;
                    animationSpeed = 100;
                }
            }
            else {
                if (x < SCREEN_WIDTH - WALL_WIDTH - PLAYER_WIDTH) {
                    x = SCREEN_WIDTH - WALL_WIDTH - PLAYER_WIDTH;
                    isAttached = true;
                    velocityY = 0;
                    isJumping = false;
                    animationSpeed = 100;
                }
            }
        }

        x = onLeftWall ? WALL_WIDTH : SCREEN_WIDTH - WALL_WIDTH - PLAYER_WIDTH;

        if (y > SCREEN_HEIGHT) {
            reset();
        }
    }

    void reset() {
        y = SCREEN_HEIGHT - 100;
        x = WALL_WIDTH;
        velocityY = 0;
        onLeftWall = true;
        isJumping = false;
        isAttached = true;
        score = 0;
        currentFrame = 0;
        animationSpeed = 100;
    }

    void draw() {
        SDL_Rect destRect = { x, y, PLAYER_WIDTH, PLAYER_HEIGHT };
        SDL_RendererFlip flip = onLeftWall ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;
        SDL_RenderCopyEx(renderer, ninjaTexture, &animationFrames[currentFrame], &destRect, 0, nullptr, flip);
    }

    SDL_Rect getRect() const {
        return { x, y, PLAYER_WIDTH, PLAYER_HEIGHT };
    }
};

SDL_Texture* loadTexture(const char* path) {
    SDL_Surface* surf = IMG_Load(path);
    if (!surf) {
        std::cerr << "IMG_Load failed: " << IMG_GetError() << "\n";
        return nullptr;
    }

    SDL_Surface* converted = SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_RGBA8888, 0);
    SDL_FreeSurface(surf);

    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, converted);
    SDL_FreeSurface(converted);
    return tex;
}

bool init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL init failed: " << SDL_GetError() << "\n";
        return false;
    }
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        std::cerr << "SDL_image init failed: " << IMG_GetError() << "\n";
        return false;
    }
    window = SDL_CreateWindow("NinJump Clone",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH, SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Window creation failed: " << SDL_GetError() << "\n";
        return false;
    }
    renderer = SDL_CreateRenderer(window, -1,
        SDL_RENDERER_ACCELERATED |
        SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        std::cerr << "Renderer creation failed: " << SDL_GetError() << "\n";
        return false;
    }
    return true;
}

void close() {
    SDL_DestroyTexture(backgroundTexture);
    SDL_DestroyTexture(ninjaTexture);
    SDL_DestroyTexture(wallTexture);
    SDL_DestroyTexture(platformTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
}

bool checkCollision(const SDL_Rect& a, const SDL_Rect& b) {
    return (a.x < b.x + b.w &&
        a.x + a.w > b.x &&
        a.y < b.y + b.h &&
        a.y + a.h > b.y);
}

void renderScore(int score, TTF_Font* font, SDL_Renderer* renderer) {
    SDL_Color color = { 255, 255, 255 };
    std::string scoreText = "Score: " + std::to_string(score);
    SDL_Surface* surface = TTF_RenderText_Solid(font, scoreText.c_str(), color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect rect = { 10, 10, surface->w, surface->h };
    SDL_FreeSurface(surface);
    SDL_RenderCopy(renderer, texture, nullptr, &rect);
    SDL_DestroyTexture(texture);
}

void renderGameOver(TTF_Font* font, SDL_Renderer* renderer) {
    SDL_Color color = { 255, 0, 0 };
    SDL_Surface* surface = TTF_RenderText_Solid(font, "GAME OVER", color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect rect = { SCREEN_WIDTH/2 - surface->w/2, SCREEN_HEIGHT/2 - surface->h/2, surface->w, surface->h };
    SDL_FreeSurface(surface);
    SDL_RenderCopy(renderer, texture, nullptr, &rect);
    SDL_DestroyTexture(texture);
}

int main(int argc, char* args[]) {
    if (!init()) return -1;

    wallTexture = loadTexture("wall.png");
    backgroundTexture = loadTexture("background.png");
    ninjaTexture = loadTexture("ninja2.png");
    platformTexture = loadTexture("platform.png");

    if (!wallTexture || !backgroundTexture || !ninjaTexture || !platformTexture) {
        close();
        return -1;
    }

    if (TTF_Init() == -1) {
        std::cerr << "TTF_Init failed: " << TTF_GetError() << "\n";
        close();
        return -1;
    }
    TTF_Font* font = TTF_OpenFont("arial.ttf", 24);
    if (!font) {
        std::cerr << "Failed to load font: " << TTF_GetError() << "\n";
        close();
        return -1;
    }

    Player player;
    std::vector<Platform> platforms;
    srand((unsigned)time(nullptr));

    platforms.emplace_back(WALL_WIDTH, player.y - SCREEN_HEIGHT);

    bool quit = false;
    bool gameOver = false;
    SDL_Event e;

    Uint32 lastTime = SDL_GetTicks();
    const Uint32 frameDelay = 16;

    while (!quit) {
        Uint32 currentTime = SDL_GetTicks();
        Uint32 deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT)
                quit = true;

            if (!gameOver && e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_SPACE) {
                player.jump();
            }
        }

        if (!gameOver) {
            player.update();
            player.updateAnimation();

            for (auto& p : platforms) {
                if (checkCollision(player.getRect(), p.rect)) {
                    gameOver = true;
                    break;
                }
            }

            if (platforms.empty() || platforms.back().rect.y > -PLATFORM_HEIGHT) {
                if (rand() % 100 < 5) {
                    bool leftSide = (rand() % 2 == 0);
                    int x = leftSide ? WALL_WIDTH : SCREEN_WIDTH - WALL_WIDTH - PLATFORM_WIDTH;
                    int y = platforms.empty() ? -PLATFORM_HEIGHT : platforms.back().rect.y - 150;
                    platforms.emplace_back(x, y);
                }
            }

            if (!platforms.empty() && platforms.front().rect.y > SCREEN_HEIGHT) {
                platforms.erase(platforms.begin());
                player.score++;
            }

            for (auto& p : platforms) {
                p.rect.y += 3;
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        SDL_Rect bgRect = { WALL_WIDTH, 0, SCREEN_WIDTH - 2 * WALL_WIDTH, SCREEN_HEIGHT };
        SDL_RenderCopy(renderer, backgroundTexture, nullptr, &bgRect);

        SDL_Rect leftWall = { 0, 0, WALL_WIDTH, SCREEN_HEIGHT };
        SDL_Rect rightWall = { SCREEN_WIDTH - WALL_WIDTH, 0, WALL_WIDTH, SCREEN_HEIGHT };
        SDL_RenderCopy(renderer, wallTexture, nullptr, &leftWall);
        SDL_RenderCopy(renderer, wallTexture, nullptr, &rightWall);

        for (auto& p : platforms) {
            p.draw();
        }

        player.draw();

        renderScore(player.score, font, renderer);

        if (gameOver) {
            renderGameOver(font, renderer);
        }

        SDL_RenderPresent(renderer);

        Uint32 frameTime = SDL_GetTicks() - currentTime;
        if (frameDelay > frameTime) {
            SDL_Delay(frameDelay - frameTime);
        }

        if (gameOver) {
            SDL_Delay(2000);
            quit = true;
        }
    }

    TTF_CloseFont(font);
    TTF_Quit();
    close();
    return 0;
}

#include "Game.h"
#include "constants.h"
#include <algorithm>

Game::Game() {}

Game::~Game() {
    cleanup();
}

bool Game::init() {
    if (!initSDL()) return false;

    font = TTF_OpenFont("PixelifySans.ttf", 36);
    if (!font) {
        std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
        return false;
    }

    if (!loadResources()) {
        return false;
    }

    highScore = loadHighScore();
    return true;
}

void Game::run() {
    while (running) {
        handleEvents();
        update();
        render();
        SDL_Delay(16);
    }
}

bool Game::initSDL() {
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

bool Game::loadResources() {
    textures.background = loadTexture("background.png");
    textures.ninja = loadTexture("ninja.png");
    textures.wall = loadTexture("wall.png");
    textures.platform = loadTexture("platform.png");
    textures.heart = loadTexture("heart.gif");
    textures.menu = loadTexture("menu.png");
    textures.gameOver = loadTexture("background.png");
    textures.pause = loadTexture("pause.png");
    textures.shuriken = loadTexture("shuriken.png");
    textures.enemy = loadTexture("enemy.png");

    sounds.jump = Mix_LoadWAV("jump.wav");
    sounds.hit = Mix_LoadWAV("hit.wav");
    sounds.loseLife = Mix_LoadWAV("lose_life.wav");
    sounds.gameOver = Mix_LoadWAV("game_over.wav");
    sounds.kill[0] = Mix_LoadWAV("kill1.wav");
    sounds.kill[1] = Mix_LoadWAV("kill2.wav");
    sounds.kill[2] = Mix_LoadWAV("kill3.wav");
    sounds.kill[3] = Mix_LoadWAV("kill4.wav");
    sounds.kill[4] = Mix_LoadWAV("kill5.wav");
    sounds.enemySpawn = Mix_LoadWAV("spawn.wav");

    if (!textures.background || !textures.ninja || !textures.wall ||
        !textures.platform || !textures.heart || !textures.menu ||
        !textures.gameOver || !textures.pause ||
        !sounds.jump || !sounds.hit || !sounds.loseLife || !sounds.gameOver ||
        !sounds.kill || !sounds.enemySpawn) {
        return false;
    }

    return true;
}

void Game::cleanup() {
    SDL_DestroyTexture(textures.background);
    SDL_DestroyTexture(textures.ninja);
    SDL_DestroyTexture(textures.shuriken);
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
    SDL_DestroyTexture(textures.enemy);
    for (int i = 0; i < 5; i++) {
        Mix_FreeChunk(sounds.kill[i]);
    }
    Mix_FreeChunk(sounds.enemySpawn);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    TTF_CloseFont(font);

    Mix_Quit();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

void Game::handleShurikens() {
    for (auto& shuriken : player.shurikens) {
        shuriken.update();
    }

    player.shurikens.erase(
        std::remove_if(player.shurikens.begin(), player.shurikens.end(),
            [](const Shuriken& s) { return !s.isActive(); }),
        player.shurikens.end());
}

void Game::handleEnemies() {
    spawnEnemies();

    for (auto& enemy : enemies) {
        enemy.update(platformSpeed);
    }

    for (auto& shuriken : player.shurikens) {
        if (!shuriken.isActive()) continue;

        for (auto& enemy : enemies) {
            if (enemy.isActive() && checkCollision(shuriken.getRect(), enemy.getRect())) {
                shuriken.deactivate();
                enemy.takeDamage();
                if (enemy.isDead()) {
                    updateKillStreak(true);
                    player.score += 10;
                }
            }
        }
    }

    for (auto& enemy : enemies) {
        if (enemy.isActive() && checkCollision(player.getRect(), enemy.getRect())) {
            if (!player.isInvincible) {
                player.lives--;
                player.isInvincible = true;
                player.invincibleTime = SDL_GetTicks() + PLAYER_INVINCIBLE_TIME;

                Mix_PlayChannel(-1, sounds.hit, 0);

                if (player.lives <= 0) {
                    Mix_PlayChannel(-1, sounds.gameOver, 0);
                    gameState = GameState::GAME_OVER;
                }
            }
        }
    }

    enemies.erase(
        std::remove_if(enemies.begin(), enemies.end(),
            [](const Enemy& e) { return !e.isActive(); }),
        enemies.end());
}

void Game::handleEvents() {
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
                    } else if (event.key.keysym.sym == SDLK_s) {
                        player.throwShuriken();
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
}

void Game::updateKillStreak(bool killedEnemy) {
    Uint32 currentTime = SDL_GetTicks();

    if (killedEnemy) {
        if (currentTime - lastKillTime > 4000) {
            killStreak = 0;
        }

        killStreak++;
        lastKillTime = currentTime;

        if (killStreak > 0 && killStreak <= 5) {
            Mix_PlayChannel(-1, sounds.kill[killStreak-1], 0);
        }
    }
    else if (currentTime - lastKillTime > 2000) {
        killStreak = 0;
    }
}

void Game::update() {
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

        handleShurikens();
        spawnPlatform();
        handleEnemies();
        updateKillStreak(false);

        if (!platforms.empty() && platforms.front().rect.y > SCREEN_HEIGHT) {
            platforms.erase(platforms.begin());
        }

        if (platformSpeed < MAX_PLATFORM_SPEED) {
            platformSpeed += SPEED_INCREASE_RATE;
        }
    }
}

void Game::render() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_Rect bgRect = { WALL_WIDTH, 0, SCREEN_WIDTH - 2 * WALL_WIDTH, SCREEN_HEIGHT };
    SDL_RenderCopy(renderer, textures.background, nullptr, &bgRect);

    SDL_Rect leftWall = { 0, 0, WALL_WIDTH, SCREEN_HEIGHT };
    SDL_Rect rightWall = { SCREEN_WIDTH - WALL_WIDTH, 0, WALL_WIDTH, SCREEN_HEIGHT };
    SDL_RenderCopy(renderer, textures.wall, nullptr, &leftWall);
    SDL_RenderCopy(renderer, textures.wall, nullptr, &rightWall);

    for (auto& shuriken : player.shurikens) {
        shuriken.render(renderer, textures.shuriken);
    }

    for (auto& enemy : enemies) {
        enemy.render(renderer, textures.enemy);
    }

    for (auto& platform : platforms) {
        platform.render(renderer, textures.platform);
    }

    player.render(renderer, textures.ninja);

    if (gameState == GameState::PLAYING) {
        renderHUD();
    }

    switch (gameState) {
        case GameState::MENU:
            renderMenu();
            break;

        case GameState::PAUSED:
            renderPause();
            break;

        case GameState::GAME_OVER:
            renderGameOver();
            break;

        default:
            break;
    }

    SDL_RenderPresent(renderer);
}

void Game::renderText(SDL_Renderer* renderer, const std::string& text, SDL_Color color, int x, int y) {
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

void Game::renderCenteredText(const std::string& text, SDL_Color color, int yOffset) {
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

void Game::renderMenu() {
    SDL_RenderCopy(renderer, textures.menu, nullptr, nullptr);

    if (highScore > 0) {
        renderCenteredText("HIGH SCORE: " + std::to_string(highScore), {255, 215, 0, 255}, -100);
    }
}

void Game::renderPause() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
    SDL_Rect overlay = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    SDL_RenderFillRect(renderer, &overlay);
    SDL_RenderCopy(renderer, textures.pause, nullptr, nullptr);
}

void Game::renderGameOver() {
    SDL_RenderCopy(renderer, textures.gameOver, nullptr, nullptr);

    renderCenteredText("SCORE: " + std::to_string(player.score), {0, 0, 0, 255}, -50);

    if (player.score == highScore) {
        renderCenteredText("NEW HIGH SCORE!", {255, 215, 0, 255}, 0);
    }

    renderCenteredText("M - MENU", {0, 0, 0, 255}, 50);
    renderCenteredText("R - RESTART", {0, 0, 0, 255}, 100);
    renderCenteredText("ESC - QUIT", {0, 0, 0, 255}, 150);
}

void Game::renderHUD() {
    for (int i = 0; i < player.lives; ++i) {
        SDL_Rect heartRect = { 10 + i * (HEART_SIZE + HEART_PADDING), 10, HEART_SIZE, HEART_SIZE };
        SDL_RenderCopy(renderer, textures.heart, nullptr, &heartRect);
    }
    renderText(renderer, "Score: " + std::to_string(player.score), {0, 0, 0, 255}, 10, 50);
}

void Game::spawnPlatform() {
    if (platforms.empty() || platforms.back().rect.y > rand() % PLATFORM_SPAWN_GAP_MIN + PLATFORM_SPAWN_GAP_MAX) {
        bool leftSide = (rand() % 2 == 0);
        int x = leftSide ? WALL_WIDTH : SCREEN_WIDTH - WALL_WIDTH - PLATFORM_WIDTH;
        int y = platforms.empty() ? SCREEN_HEIGHT : platforms.back().rect.y - (rand() % PLATFORM_SPAWN_RANGE_MIN + PLATFORM_SPAWN_RANGE_MAX);
        platforms.emplace_back(x, y);
    }
}

void Game::spawnEnemies() {
    Uint32 currentTime = SDL_GetTicks();
    if (currentTime - lastSpawnTime > SPAWN_INTERVAL) {
        lastSpawnTime = currentTime;

        int enemyCount = 2 + rand() % 5;
        for (int i = 0; i < enemyCount; i++) {
            bool leftSide = rand() % 2 == 0;
            int x = leftSide ? WALL_WIDTH : SCREEN_WIDTH - WALL_WIDTH - ENEMY_WIDTH;
            int y = -ENEMY_HEIGHT - (i * 50);
            enemies.emplace_back(x, y, leftSide);
        }

        Mix_PlayChannel(-1, sounds.enemySpawn, 0);
    }
}

bool Game::checkCollision(const SDL_Rect& a, const SDL_Rect& b) {
    return (a.x < b.x + b.w && a.x + a.w > b.x && a.y < b.y + b.h && a.y + a.h > b.y);
}

int Game::loadHighScore() {
    int highScore = 0;
    std::ifstream file(HIGH_SCORE_FILE, std::ios::binary);
    if (file) {
        file.read(reinterpret_cast<char*>(&highScore), sizeof(highScore));
        file.close();
    }
    return highScore;
}

void Game::saveHighScore(int score) {
    std::ofstream file(HIGH_SCORE_FILE, std::ios::binary);
    if (file) {
        file.write(reinterpret_cast<const char*>(&score), sizeof(score));
        file.close();
    }
}

SDL_Texture* Game::loadTexture(const std::string& path) {
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

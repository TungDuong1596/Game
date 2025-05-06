#pragma once
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include "Player.h"
#include "GameTextures.h"
#include "GameSounds.h"
#include "Platform.h"
#include "constants.h"
#include "enemy.h"

enum class GameState { MENU, PLAYING, PAUSED, GAME_OVER, QUIT };

class Game {
public:
    Game();
    ~Game();

    bool init();
    void run();
    void handleShurikens();
    void handleEnemies();
    void spawnEnemies();
    void updateKillStreak(bool killedEnemy);

private:
    bool initSDL();
    bool loadResources();
    void cleanup();
    void handleEvents();
    void update();
    void render();
    void renderText(SDL_Renderer* renderer, const std::string& text, SDL_Color color, int x, int y);
    void renderCenteredText(const std::string& text, SDL_Color color, int yOffset);
    void renderMenu();
    void renderPause();
    void renderGameOver();
    void renderHUD();
    void spawnPlatform();
    bool checkCollision(const SDL_Rect& a, const SDL_Rect& b);
    int loadHighScore();
    void saveHighScore(int score);

    SDL_Texture* loadTexture(const std::string& path);
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    TTF_Font* font = nullptr;
    GameTextures textures;
    GameSounds sounds;
    Player player;
    std::vector<Platform> platforms;
    GameState gameState = GameState::MENU;
    float platformSpeed = INITIAL_PLATFORM_SPEED;
    int highScore = 0;
    bool running = true;
    std::vector<Enemy> enemies;
    int killStreak;
    Uint32 lastKillTime;
    Uint32 lastSpawnTime;
    const Uint32 SPAWN_INTERVAL = 5000;
    float backgroundOffset;
    const float BACKGROUND_SCROLL_SPEED = 0.5f;
};

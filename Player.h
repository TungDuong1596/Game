#pragma once
#include <SDL.h>
#include "GameSounds.h"
#include <string>
#include "constants.h"
#include <vector>
#include "shuriken.h"

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
    std::vector<Shuriken> shurikens;
    static const int MAX_SHURIKENS = 7;
    Uint32 invincibleTime;

    Player();
    void jump(GameSounds& sounds);
    void throwShuriken();
    void update();
    void reset();
    void resetPosition();
    void render(SDL_Renderer* renderer, SDL_Texture* texture);
    SDL_Rect getRect() const;

private:
    int targetY;
    Uint32 lastMultiplierIncreaseTime;
    Uint32 lastScoreUpdateTime;
};


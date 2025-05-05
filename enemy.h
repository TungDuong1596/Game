#pragma once
#include <SDL.h>
#include "constants.h"

class Enemy {
public:
    Enemy(int x, int y, bool isLeftSide);
    void update(float speed);
    void render(SDL_Renderer* renderer, SDL_Texture* texture);
    SDL_Rect getRect() const;
    bool isActive() const;
    void takeDamage();
    bool isDead() const;

private:
    SDL_Rect rect;
    bool active;
    bool leftSide;
    int health;
    static const int ENEMY_WIDTH = 30;
    static const int ENEMY_HEIGHT = 30;
};


#include "enemy.h"

Enemy::Enemy(int x, int y, bool isLeftSide) :
    leftSide(isLeftSide), active(true), health(2) {  // 2 phi tiêu mới chết
    rect = { x, y, ENEMY_WIDTH, ENEMY_HEIGHT };
}

void Enemy::update(float speed) {
    rect.y += static_cast<int>(speed);  // Di chuyển xuống cùng tốc độ platform
}

void Enemy::render(SDL_Renderer* renderer, SDL_Texture* texture) {
    if (active) {
        SDL_RenderCopy(renderer, texture, nullptr, &rect);
    }
}

void Enemy::takeDamage() {
    health--;
    if (health <= 0) {
        active = false;
    }
}

SDL_Rect Enemy::getRect() const { return rect; }
bool Enemy::isActive() const { return active; }
bool Enemy::isDead() const { return !active && health <= 0; }

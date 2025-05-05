#include "Player.h"
#include "constants.h"

Player::Player() : x(WALL_WIDTH), y(SCREEN_HEIGHT - 100 - PLAYER_HEIGHT), velocityY(0),
                  onLeftWall(true), isJumping(false), isAttached(true), score(0),
                  scoreMultiplier(1.0f), lives(3), isInvincible(false) {
    targetY = y;
    lastMultiplierIncreaseTime = SDL_GetTicks();
    lastScoreUpdateTime = SDL_GetTicks();
}

void Player::jump(GameSounds& sounds) {
    if (isAttached) {
        isAttached = false;
        onLeftWall = !onLeftWall;
        velocityY = JUMP_FORCE;
        isJumping = true;
        targetY = y;
        Mix_PlayChannel(-1, sounds.jump, 0);
    }
}

void Player::throwShuriken() {
    if (isAttached && shurikens.size() < MAX_SHURIKENS) {
        int centerX = x + PLAYER_WIDTH/2;
        int centerY = y + PLAYER_HEIGHT/2;
        shurikens.emplace_back(centerX, centerY);
    }
}

void Player::update() {
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

void Player::reset() {
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

void Player::resetPosition() {
    y = SCREEN_HEIGHT - 100 - PLAYER_HEIGHT;
    targetY = y;
    x = onLeftWall ? WALL_WIDTH : SCREEN_WIDTH - WALL_WIDTH - PLAYER_WIDTH;
    velocityY = 0;
    isAttached = true;
    isJumping = false;
    isInvincible = true;
    invincibleTime = SDL_GetTicks() + PLAYER_INVINCIBLE_TIME;
}

void Player::render(SDL_Renderer* renderer, SDL_Texture* texture) {
    SDL_Rect destRect = { x, y, PLAYER_WIDTH, PLAYER_HEIGHT };
    SDL_RendererFlip flip = onLeftWall ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;
    SDL_RenderCopyEx(renderer, texture, nullptr, &destRect, 0, nullptr, flip);
}

SDL_Rect Player::getRect() const {
    return { x, y, PLAYER_WIDTH, PLAYER_HEIGHT };
}

#pragma once
#include <string>

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
const int SHURIKEN_SPEED = 10;
const int SHURIKEN_WIDTH = PLAYER_WIDTH / 2;
const int SHURIKEN_HEIGHT = PLAYER_HEIGHT / 2;
constexpr int ENEMY_WIDTH = 30;
constexpr int ENEMY_HEIGHT = 30;
constexpr int ENEMY_HEALTH = 2;

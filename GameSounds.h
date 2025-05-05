#pragma once
#include <SDL_mixer.h>

struct GameSounds {
    Mix_Chunk* jump = nullptr;
    Mix_Chunk* hit = nullptr;
    Mix_Chunk* loseLife = nullptr;
    Mix_Chunk* gameOver = nullptr;
    Mix_Chunk* kill[5];
    Mix_Chunk* enemySpawn = nullptr;
};

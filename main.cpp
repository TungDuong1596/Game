#include "Game.h"

int main(int argc, char* args[]) {
    Game game;
    if (game.init()) {
        game.run();
    }
    return 0;
}


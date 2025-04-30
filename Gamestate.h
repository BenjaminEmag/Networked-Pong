#pragma once
#include "raylib.h"
#include "socklib.h"
#include <iostream>

const int BALLS = 10;
const int POWERUPS = 10;

enum class InputCommand {
    None,
    Up,
    Down,
    Restart
};

struct PowerupData {
    Vector2 position;
    int type; // 0 = Multiball 1 = Bigger Paddle 2 = Speed Boost
    bool active;
};

class GameState {
public:
    static const int MAX_BALLS = BALLS;
    static const int MAX_POWERUPS = POWERUPS;

    Vector2 paddlePositions[2];
    float paddleHeights[2];
    Vector2 ballPositions[MAX_BALLS];
    Vector2 ballVelocities[MAX_BALLS];
    bool ballActive[MAX_BALLS];
    int scores[2];
    PowerupData powerups[MAX_POWERUPS];

    bool gameOver = false;
    int winner = -1;

    GameState();

    void Serialize(std::ostream& os) const;
    void Deserialize(std::istream& is);
};

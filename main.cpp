#include "raylib.h"
#include "game.h"

int main() {
    const int screenWidth = 800;
    const int screenHeight = 600;

    InitWindow(screenWidth, screenHeight, "Network Pong");
    SetTargetFPS(60);

    Game game(screenWidth, screenHeight);

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_R)) 
            game.Reset();
        

        game.Update();

        BeginDrawing();
        DrawFPS(20, 20);
        ClearBackground(BLACK);
        game.Draw();
        EndDrawing();
    }

    CloseWindow();
    return 0;
}

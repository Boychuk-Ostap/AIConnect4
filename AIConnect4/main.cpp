#include "raylib.h"
#include "game.h"

int main()
{
    InitWindow(1920, 1080, "Raylib test");
    SetTargetFPS(60);

    Game game;

    while (!WindowShouldClose())
    {
        game.Update();
        BeginDrawing();
        game.Draw();
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
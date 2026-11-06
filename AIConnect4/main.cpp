#include "raylib.h"

int main()
{
    InitWindow(800, 450, "Raylib test");
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText("Raylib is working!", 250, 200, 30, DARKGREEN);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
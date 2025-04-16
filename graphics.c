#include "raylib.h"

#define WIDTH  800
#define HEIGHT 600

int main(void) {
    InitWindow(WIDTH, HEIGHT, "Computer Graphics");

    SetTargetFPS(120);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(GetColor(0x181818FF));
        EndDrawing();
    }

    return 0;
}

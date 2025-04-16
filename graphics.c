#include <stdint.h>
#include <stdlib.h>

#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "nob.h"

#include "raylib.h"

#define WIDTH  800
#define HEIGHT 600

typedef struct {
    uint32_t *pixels;
    int width;
    int height;
} Canvas;


void put_pixel(Canvas *canvas, int x, int y, uint32_t color) {
    canvas->pixels[y*canvas->width+x] = color;
}

int main(void) {
    InitWindow(WIDTH, HEIGHT, "Computer Graphics");

    Canvas canvas = {0};
    canvas.width = 10;
    canvas.height = 10;
    canvas.pixels = calloc(sizeof(Color), canvas.width*canvas.height);

    for (int y = 0; y < canvas.height; y++) {
        for (int x = 0; x < canvas.width; x++) {
            put_pixel(&canvas, x, y, 0xFF0000FF);
        }
    }
    Image image = {0};
    image.data = canvas.pixels;
    image.width = canvas.width;
    image.height = canvas.height;
    image.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
    image.mipmaps = 1;
    Texture2D texture = LoadTextureFromImage(image);
    UnloadImage(image);

    SetTargetFPS(120);
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(GetColor(0x181818FF));
        DrawTexture(texture, WIDTH/2, HEIGHT/2, WHITE);
        EndDrawing();
    }

    UnloadTexture(texture);
    CloseWindow();
    return 0;
}

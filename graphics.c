#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "nob.h"

#include "raylib.h"

#define RUN_RAYLIB
#undef RUN_RAYLIB

#ifdef RUN_RAYLIB
    #define WIDTH  800
    #define HEIGHT 600
#else
    #define WIDTH  255*4
    #define HEIGHT 255*4
#endif // RUN_RAYLIB

typedef struct {
    uint32_t *pixels;
    int width;
    int height;
} Canvas;

void put_pixel(Canvas *canvas, int x, int y, uint32_t color) {
    canvas->pixels[y*canvas->width+x] = color;
}

Texture2D canvas_to_texture(Canvas *canvas) {
    Image image = {0};
    image.data = canvas->pixels;
    image.width = canvas->width;
    image.height = canvas->height;
    image.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
    image.mipmaps = 1;
    Texture2D texture = LoadTextureFromImage(image);
    UnloadImage(image);
    return texture;
}

void canvas_to_ppm_file(Canvas *canvas) {
    FILE *f = fopen("canvas.ppm", "w");
    if (f == NULL) {
        fprintf(stderr, "ERROR: Could not open out.ppm\n");
        return;
    }

    fprintf(f, "P6\n");
    fprintf(f, "%d %d\n", canvas->width, canvas->height);
    fprintf(f, "255\n");
    for (int y = 0; y < canvas->height; y++) {
        for (int x = 0; x < canvas->width; x++) {
            uint32_t c = canvas->pixels[y*canvas->width+x];
            uint8_t r = ((c >> 8*0) & 0xFF);
            uint8_t g = ((c >> 8*1) & 0xFF);
            uint8_t b = ((c >> 8*2) & 0xFF);
            fputc(r, f);
            fputc(g, f);
            fputc(b, f);
        }
    }
}


int main(void) {

#ifdef RUN_RAYLIB
    InitWindow(WIDTH, HEIGHT, "Computer Graphics");
#endif // RUN_RAYLIB

    Canvas canvas = {0};
#ifdef RUN_RAYLIB
    canvas.width = WIDTH/2;
    canvas.height = HEIGHT/2;
#else
    canvas.width = WIDTH;
    canvas.height = HEIGHT;
#endif // RUN_RAYLIB
    canvas.pixels = calloc(sizeof(Color), canvas.width*canvas.height);

#ifdef RUN_RAYLIB
    Texture2D texture = canvas_to_texture(&canvas);
#endif // RUN_RAYLIB

    for (int y = 0; y < canvas.height; y++) {
        for (int x = 0; x < canvas.width; x++) {
            uint8_t r = (255+x/((y*y)+1))%255;
            uint8_t g = (122+x*y)%255;
            uint8_t b = 0;
            uint8_t a = 255;
            put_pixel(&canvas, x, y, (uint32_t)((r>>(8*0))|(g<<(8*1))|(b<<(8*2))|(a<<(8*3))));
        }
    }

    canvas_to_ppm_file(&canvas);

#ifdef RUN_RAYLIB
    UpdateTexture(texture, canvas.pixels);
    SetTargetFPS(120);
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(GetColor(0x181818FF));
        DrawTexture(texture, WIDTH/2-canvas.width/2, HEIGHT/2-canvas.height/2, WHITE);
        EndDrawing();
    }

    UnloadTexture(texture);
    CloseWindow();
#endif // RUN_RAYLIB

    return 0;
}

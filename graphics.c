#include <stdio.h>
#include <stdint.h>
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

uint8_t clamp_color(int v) {
    if (v < 0) return 0;
    if (v > 255) return 255;
    return (uint8_t)v;
}

#define to_c_with_alpha(r, g, b, a) ((r>>(8*0)) | (g<<(8*1)) | (b<<(8*2)) | (a<<(8*3)))
#define to_c(r, g, b) ((r>>(8*0)) | (g<<(8*1)) | (b<<(8*2)) | (0xFF<<(8*3)))
#define color_r(c) ((c >> 8*0) & 0xFF)
#define color_g(c) ((c >> 8*1) & 0xFF)
#define color_b(c) ((c >> 8*2) & 0xFF)
#define color_a(c) ((c >> 8*3) & 0xFF)
#define color_add(c, c2) to_c(clamp_color(color_r(c)+color_r(c2)), clamp_color(color_g(c)+color_g(c2)), clamp_color(color_b(c)+color_b(c2)))
#define color_mult(c, k) to_c(clamp_color((color_r(c)*k)), clamp_color((color_g(c)*k)), clamp_color((color_b(c)*k)))

void put_pixel(Canvas *canvas, int x, int y, uint32_t color) {
    canvas->pixels[y*canvas->width+x] = color;
}

void PutPixel(Canvas *canvas, int x, int y, uint32_t color) {
    assert(-canvas->width/2 <= x && x <= canvas->width/2 && "Overflow x");
    assert(-canvas->height/2 <= y && y <= canvas->height/2 && "Overflow y");
    put_pixel(canvas, (canvas->width/2)+x, (canvas->height/2)-y, color);
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
            fputc(color_r(c), f);
            fputc(color_g(c), f);
            fputc(color_b(c), f);
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
            put_pixel(&canvas, x, y, to_c_with_alpha(r, g, b, a));
        }
    }

    for (int y = -75; y < 25; y++) {
        for (int x = -75; x < 25; x++) {
            PutPixel(&canvas, x, y, color_add(to_c(255, 0, 0), to_c(0, 255, 0)));
        }
    }
    for (int y = -50; y < 50; y++) {
        for (int x = -50; x < 50; x++) {
            PutPixel(&canvas, x, y, color_mult(to_c(192, 64, 32), 2));
        }
    }
    for (int y = -25; y < 75; y++) {
        for (int x = -25; x < 75; x++) {
            PutPixel(&canvas, x, y, to_c(192, 64, 32));
        }
    }

#ifndef RUN_RAYLIB
    canvas_to_ppm_file(&canvas);
#endif // RUN_RAYLIB

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

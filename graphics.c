#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>

#include "raylib.h"
#include "raymath.h"

#define NOB_IMPLEMENTATION
#include "nob.h"

#define WIDTH  800
#define HEIGHT 600

typedef struct {
    uint32_t *pixels;
    int width;
    int height;
} Canvas;

typedef enum {
    SCENE_OBJECT_SPHERE = 1,
} SceneObjectType;

typedef struct {
    float radius;
    Vector3 center;
    uint32_t color;
} Sphere;

typedef struct {
    SceneObjectType type;
    union {
        Sphere sphere;
    } obj;
} SceneObject;

typedef struct {
    SceneObject *items;
    size_t count;
    size_t capacity;
} Scene;

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
    assert(0 <= x && x < canvas->width && "Overflow x");
    assert(0 <= y && y < canvas->height && "Overflow y");
    canvas->pixels[y*canvas->width+x] = color;
}

void PutPixel(Canvas *canvas, int x, int y, uint32_t color) {
    assert(-canvas->width/2 <= x && x < canvas->width/2 && "Overflow x");
    assert(-canvas->height/2 <= y && y < canvas->height/2 && "Overflow y");
    put_pixel(canvas, (canvas->width/2)+x, (canvas->height/2)-y-1, color);
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

Vector3 canvas_to_viewport(Canvas *canvas, int vw, int vh, int x, int y, int d) {
    return (Vector3){
        .x = (float)x*(float)vw/(float)canvas->width,
        .y = (float)y*(float)vh/(float)canvas->height,
        .z = d
    };
}

#define T_MAX FLT_MAX
Vector2 IntersectRaySphere(Vector3 origin, Vector3 direction, Sphere sphere) {
    Vector3 CO = Vector3Subtract(origin, sphere.center);

    float a = Vector3DotProduct(direction, direction);
    float b = 2*Vector3DotProduct(CO, direction);
    float c = Vector3DotProduct(CO, CO) - sphere.radius * sphere.radius;

    float discriminant = b*b - 4*a*c;
    if (discriminant < 0) {
        return (Vector2){T_MAX, T_MAX};
    }

    float t1 = (-b + sqrt(discriminant)) / (2*a);
    float t2 = (-b - sqrt(discriminant)) / (2*a);
    return (Vector2){t1, t2};
}

uint32_t trace_ray(Scene *scene, Vector3 origin, Vector3 direction, float t_min, float t_max) {
    float closest_t = t_max;
    uint32_t color = to_c(0x18, 0x18, 0x18);

    for (size_t i = 0; i < scene->count; i++) {
        switch (scene->items[i].type) {
            case SCENE_OBJECT_SPHERE: {
                Sphere sphere = scene->items[i].obj.sphere;
                Vector2 ts = IntersectRaySphere(origin, direction, sphere);
                float t1 = ts.x;
                float t2 = ts.y;
                if (t_min < t1 && t1 < t_max && t1 < closest_t) {
                    closest_t = t1;
                    color = sphere.color;
                }
                if (t_min < t2 && t2 < t_max && t2 < closest_t) {
                    closest_t = t2;
                    color = sphere.color;
                }
            } break;
            default:
                NOB_UNREACHABLE("Only spheres");
                break;
        }
    }
    return color;
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
    Canvas canvas = {0};
    canvas.width = WIDTH/2;
    canvas.height = HEIGHT/2;
    canvas.pixels = calloc(sizeof(uint32_t), canvas.width*canvas.height);

#if 1
    for (int i = 0; i < canvas.width; i++) {
        for (int j = 0; j < canvas.height; j++) {
            if (i == 0 || i + 1 == canvas.width || j == 0 || j + 1 == canvas.height) {
                put_pixel(&canvas, i, j, to_c(255, 255, 255));
            }
        }
    }
#endif

#if 0
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
    for (int y = -canvas.height/2; y < canvas.height/2; y++) {
        for (int x = -canvas.width/2; x < canvas.width/2; x++) {
            uint8_t r = (255+x/((y*y)+1))%255;
            uint8_t g = (122+x*y)%255;
            uint8_t b = 0;
            uint8_t a = 255;
            PutPixel(&canvas, x, y, to_c_with_alpha(r, g, b, a));
        }
    }
#endif

#if 1
    Vector3 camera = {0, 0, 0};
    int vw = 1;
    int vh = 1;
    int d = 1;
    Scene scene = {0};

    nob_da_append(&scene, ((SceneObject) {
        .type = SCENE_OBJECT_SPHERE,
        .obj = {
            (Sphere){
                .radius = 1,
                .center = (Vector3){0, -1, 3},
                .color = to_c(255, 0, 0)
            }
        }
    }));
    nob_da_append(&scene, ((SceneObject) {
        .type = SCENE_OBJECT_SPHERE,
        .obj = {
            (Sphere){
                .radius = 1,
                .center = (Vector3){2, 0, 4},
                .color = to_c(0, 0, 255)
            }
        }
    }));
    nob_da_append(&scene, ((SceneObject) {
        .type = SCENE_OBJECT_SPHERE,
        .obj = {
            (Sphere){
                .radius = 1,
                .center = (Vector3){-2, 0, 4},
                .color = to_c(255, 255, 0)
            }
        }
    }));
    nob_da_append(&scene, ((SceneObject) {
        .type = SCENE_OBJECT_SPHERE,
        .obj = {
            (Sphere){
                .radius = 5000,
                .center = (Vector3){0, -5001, 0},
                .color = to_c(255, 255, 255)
            }
        }
    }));

    for (int y = -canvas.height/2; y < canvas.height/2; y++) {
        for (int x = -canvas.width/2; x < canvas.width/2; x++) {
            Vector3 direction = canvas_to_viewport(&canvas, vw, vh, x, y, d);
            uint32_t color = trace_ray(&scene, camera, direction, 1, T_MAX);
            PutPixel(&canvas, x, y, color);
        }
    }
#endif

#if 1
    canvas_to_ppm_file(&canvas);
#endif

#if 0
    InitWindow(WIDTH, HEIGHT, "Computer Graphics");
    Texture2D texture = canvas_to_texture(&canvas);
    SetTargetFPS(120);
    while (!WindowShouldClose()) {
        BeginDrawing();
            DrawFPS(50, 50);
            ClearBackground(GetColor(0x181818FF));
            DrawTexture(texture, 0, 0, WHITE);
        EndDrawing();
    }


    UnloadTexture(texture);
    CloseWindow();
#endif

    free(canvas.pixels);

    return 0;
}

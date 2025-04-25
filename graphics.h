#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <float.h>
#include <assert.h>

#include "raylib.h"
#include "raymath.h"


#define UNUSED(value) (void)(value)
#define TODO(message) do { fprintf(stderr, "%s:%d: TODO: %s\n", __FILE__, __LINE__, message); abort(); } while(0)
#define UNREACHABLE(message) do { fprintf(stderr, "%s:%d: UNREACHABLE: %s\n", __FILE__, __LINE__, message); abort(); } while(0)

typedef struct {
    uint32_t *pixels;
    int width;
    int height;
} Canvas;

typedef enum {
    SCENE_OBJECT_SPHERE = 1,
    SCENE_OBJECT_LIGHT = 2,
} SceneObjectType;

typedef struct {
    float radius;
    Vector3 center;
    uint32_t color;
} Sphere;

typedef enum {
    LIGHT_TYPE_AMBIENT = 1,
    LIGHT_TYPE_POINT = 2,
    LIGHT_TYPE_DIRECTIONAL = 3,
} LightType;

typedef struct {
    LightType type;
    float intensity;
    union {
        Vector3 position;
        Vector3 direction;
    };
} Light;

typedef struct {
    SceneObjectType type;
    union {
        Sphere sphere;
        Light light;
    } obj;
} SceneObject;

typedef struct {
    SceneObject *items;
    size_t count;
    size_t capacity;
} Scene;

#define T_MAX FLT_MAX

uint8_t clamp_color(int v);
void put_pixel(Canvas *canvas, int x, int y, uint32_t color);
void PutPixel(Canvas *canvas, int x, int y, uint32_t color);
Texture2D canvas_to_texture(Canvas *canvas);
Vector3 canvas_to_viewport(Canvas *canvas, float vw, float vh, float d, float x, float y);
Vector2 IntersectRaySphere(Vector3 origin, Vector3 direction, Sphere sphere);
float compute_lighting(Scene *scene, Vector3 P, Vector3 N);
uint32_t trace_ray(Scene *scene, Vector3 origin, Vector3 direction, float t_min, float t_max);
void canvas_to_ppm_file(Canvas *canvas, const char *filepath);
void render_scene(Canvas *canvas, Scene *scene, Vector3 camera, Vector2 v, float distance);

#endif // GRAPHICS_H

#ifdef GRAPHICS_IMPLEMENTATION

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

Vector3 canvas_to_viewport(Canvas *canvas, float vw, float vh, float d, float x, float y) {
    return (Vector3){
        .x = x*vw/canvas->width,
        .y = y*vh/canvas->height,
        .z = d
    };
}

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

float compute_lighting(Scene *scene, Vector3 P, Vector3 N) {
    float intensity = 0.0;
    float length_n = Vector3Length(N);
    for (size_t i = 0; i < scene->count; i++) {
        switch (scene->items[i].type) {
            case SCENE_OBJECT_SPHERE:
                continue;
            case SCENE_OBJECT_LIGHT: {
                Light light = scene->items[i].obj.light;
                Vector3 L;
                if (light.type == LIGHT_TYPE_AMBIENT) {
                    intensity += light.intensity;
                } else {
                    if (light.type == LIGHT_TYPE_DIRECTIONAL) {
                        L = Vector3Subtract(light.position, P);
                    } else {
                        assert(light.type == LIGHT_TYPE_POINT);
                        L = light.direction;
                    }
                    float n_dot_l = Vector3DotProduct(N, L);
                    if (n_dot_l > 0) {
                        intensity += light.intensity * n_dot_l/(length_n * Vector3Length(L));
                    }
                }
            } break;
            default:
                UNREACHABLE("Only spheres");
                break;
        }
    }
    return intensity;
}


uint32_t trace_ray(Scene *scene, Vector3 origin, Vector3 direction, float t_min, float t_max) {
    float closest_t = t_max;

    Sphere *closest_sphere = NULL;

    for (size_t i = 0; i < scene->count; i++) {
        switch (scene->items[i].type) {
            case SCENE_OBJECT_SPHERE: {
                Sphere *sphere = &scene->items[i].obj.sphere;
                Vector2 ts = IntersectRaySphere(origin, direction, *sphere);
                float t1 = ts.x;
                float t2 = ts.y;
                if (t_min < t1 && t1 < t_max && t1 < closest_t) {
                    closest_t = t1;
                    closest_sphere = sphere;
                }
                if (t_min < t2 && t2 < t_max && t2 < closest_t) {
                    closest_t = t2;
                    closest_sphere = sphere;
                }
            } break;
            case SCENE_OBJECT_LIGHT:
                continue;
            default:
                UNREACHABLE("Only spheres");
                break;
        }
    }

    if (closest_sphere == NULL) {
        return to_c(0x18, 0x18, 0x18);
    }

    Vector3 P = Vector3Add(origin, Vector3Scale(direction, closest_t));
    Vector3 N = Vector3Subtract(P, closest_sphere->center);
    N = Vector3Scale(N, 1.0/Vector3Length(N));
    return color_mult(closest_sphere->color, compute_lighting(scene, P, N));
}

void canvas_to_ppm_file(Canvas *canvas, const char *filepath) {
    FILE *f = fopen(filepath, "w");
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

void render_scene(Canvas *canvas, Scene *scene, Vector3 camera, Vector2 v, float distance) {
    for (int y = -canvas->height/2; y < canvas->height/2; y++) {
        for (int x = -canvas->width/2; x < canvas->width/2; x++) {
            Vector3 direction = canvas_to_viewport(canvas, v.x, v.y, distance, x, y);
            uint32_t color = trace_ray(scene, camera, direction, 1, T_MAX);
            PutPixel(canvas, x, y, color);
        }
    }
}

#endif // GRAPHICS_IMPLEMENTATION

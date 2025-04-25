#define NOB_IMPLEMENTATION
#include "nob.h"

#define GRAPHICS_IMPLEMENTATION
#include "graphics.h"

#define INTERACTIVE_MODE
//#undef INTERACTIVE_MODE

#ifdef INTERACTIVE_MODE
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#endif

#define WIDTH  800
#define HEIGHT 600

int main(void) {
    Canvas canvas = {0};
    canvas.width = WIDTH;
    canvas.height = HEIGHT;
    canvas.pixels = calloc(sizeof(uint32_t), canvas.width*canvas.height);

    Vector3 camera = {0, 0, 0};
    float vw = 1;
    float vh = 1;
    float d = 1;
    Scene scene = {0};

#if 0
    nob_da_append(&scene, ((SceneObject) {
        .type = SCENE_OBJECT_SPHERE,
        .obj = {
            .sphere = (Sphere){
                .radius = 1,
                .center = (Vector3){-1, -1, 5},
                .color = to_c(255, 0, 0)
            }
        }
    }));

    nob_da_append(&scene, ((SceneObject) {
        .type = SCENE_OBJECT_SPHERE,
        .obj = {
            .sphere = (Sphere){
                .radius = 1,
                .center = (Vector3){-2, -1.1, 4},
                .color = to_c(0, 255, 0)
            }
        }
    }));

    nob_da_append(&scene, ((SceneObject) {
        .type = SCENE_OBJECT_SPHERE,
        .obj = {
            .sphere = (Sphere){
                .radius = 1,
                .center = (Vector3){1, -1, 4},
                .color = to_c(0, 0, 255)
            }
        }
    }));
#endif

#if 1
    nob_da_append(&scene, ((SceneObject) {
        .type = SCENE_OBJECT_SPHERE,
        .obj = {
            .sphere = (Sphere){
                .radius = 1,
                .center = (Vector3){0, -1, 3},
                .color = to_c(255, 0, 0)
            }
        }
    }));

    nob_da_append(&scene, ((SceneObject) {
        .type = SCENE_OBJECT_SPHERE,
        .obj = {
            .sphere = (Sphere){
                .radius = 1,
                .center = (Vector3){-2, 0, 4},
                .color = to_c(0, 255, 0)
            }
        }
    }));

    nob_da_append(&scene, ((SceneObject) {
        .type = SCENE_OBJECT_SPHERE,
        .obj = {
            .sphere = (Sphere){
                .radius = 1,
                .center = (Vector3){2, 0, 4},
                .color = to_c(0, 0, 255)
            }
        }
    }));
#endif

    nob_da_append(&scene, ((SceneObject) {
        .type = SCENE_OBJECT_SPHERE,
        .obj = {
            .sphere = (Sphere){
                .radius = 5000,
                .center = (Vector3){0, -5001, 0},
                .color = to_c(255, 255, 0)
            }
        }
    }));

    nob_da_append(&scene, ((SceneObject) {
        .type = SCENE_OBJECT_LIGHT,
        .obj = {
            .light = (Light){
                .type = LIGHT_TYPE_AMBIENT,
                .intensity = 0.2,
            }
        }
    }));

    nob_da_append(&scene, ((SceneObject) {
        .type = SCENE_OBJECT_LIGHT,
        .obj = {
            .light = (Light){
                .type = LIGHT_TYPE_POINT,
                .intensity = 0.6,
                .position = (Vector3){2, 1, 0}
            }
        }
    }));

    nob_da_append(&scene, ((SceneObject) {
        .type = SCENE_OBJECT_LIGHT,
        .obj = {
            .light = (Light){
                .type = LIGHT_TYPE_DIRECTIONAL,
                .intensity = 0.2,
                .direction = (Vector3){1, 4, 4}
            }
        }
    }));

    nob_da_append(&scene, ((SceneObject) {
        .type = SCENE_OBJECT_SPHERE,
        .obj = {
            .sphere = (Sphere){
                .radius = 5000,
                .center = (Vector3){0, -5001, 0},
                .color = to_c(255, 255, 0)
            }
        }
    }));

    render_scene(&canvas, &scene, camera, (Vector2){vw, vh}, d);

#ifndef INTERACTIVE_MODE
    canvas_to_ppm_file(&canvas, "canvas.ppm");
#else
    InitWindow(WIDTH, HEIGHT, "Computer Graphics");
    Texture2D texture = canvas_to_texture(&canvas);
    canvas.pixels = calloc(sizeof(uint32_t), canvas.width*canvas.height);
    SetTargetFPS(120);
    bool should_update_canvas = false;
    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_S)) {
            canvas_to_ppm_file(&canvas, "saved.ppm");
        }

        if (should_update_canvas) {
            render_scene(&canvas, &scene, camera, (Vector2){vw, vh}, d);
            UpdateTexture(texture, canvas.pixels);
            should_update_canvas = false;
        }

        BeginDrawing();
        {
            ClearBackground(GetColor(0x181818FF));
            DrawTexture(texture, 0, 0, WHITE);
            DrawFPS(WIDTH-120, 50);

            int result = 0;
            int y = 24;
            result += GuiSlider((Rectangle){24,y,120,30}, "c.x", NULL, &camera.x, -2.5, 2.5);
            y += 30+2;
            result += GuiSlider((Rectangle){24,y,120,30}, "c.y", NULL, &camera.y, -2.5, 2.5);
            y += 30+2;
            result += GuiSlider((Rectangle){24,y,120,30}, "c.z", NULL, &camera.z, -2.5, 2.5);
            y += 30+2;
            result += GuiSlider((Rectangle){24,y,120,30}, "vw", NULL, &vw, -2, 4);
            y += 30+2;
            result += GuiSlider((Rectangle){24,y,120,30}, "vh", NULL, &vh, -2, 4);
            y += 30+2;
            result += GuiSlider((Rectangle){24,y,120,30}, "d", NULL, &d, 0.5, 1.5);

            if (result > 0) should_update_canvas = true;
        }
        EndDrawing();
    }

    UnloadTexture(texture);
    CloseWindow();
#endif

    free(canvas.pixels);

    return 0;
}

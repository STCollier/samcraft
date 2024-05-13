#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "engine/core/window.h"
#include "engine/core/shader.h"
#include "engine/core/camera.h"
#include "engine/core/globals.h"

#include "engine/func/player.h"
#include "engine/func/mesher.h"
#include "engine/gfx/sprite.h"
#include "engine/gfx/skybox.h"
#include "engine/gfx/text.h"

#include "engine/util/thpool.h"

#include "world/world.h"
#include "world/chunk.h"
#include "world/block.h"

int main() {
    window_create("samcraft", /*1920, 1080*/ 1600, 900);
    shader_t mainShader = shader_new("res/shaders/main.vert", "res/shaders/main.frag");
    shader_t blockOverlayShader = shader_new("res/shaders/block_overlay.vert", "res/shaders/block_overlay.frag");
    shader_t shader2D = shader_new("res/shaders/2D.vert", "res/shaders/2D.frag");
    shader_t skyShader = shader_new("res/shaders/sky.vert", "res/shaders/sky.frag");
    shader_t textShader = shader_new("res/shaders/text.vert", "res/shaders/text.frag");

    globals_init();

    mat4 projection2D;
    glm_ortho(0.0f, window.width, window.height, 0.0f, -1.0f, 1.0f, projection2D);

    shader_use(textShader);
    shader_setMat4(textShader, "projection", projection2D);
    shader_use(shader2D);
    shader_setMat4(shader2D, "projection", projection2D);

    struct Sprite2D crosshair = sprite2D_new("res/textures/crosshair.png", (ivec2){window.width / 2, window.height / 2}, 2.0f);
    struct Sprite2D uipanel = sprite2D_new("res/textures/panel.png", (ivec2){25, 25}, 0.5f);
    struct Skybox sky = skybox_new();

    struct Font robotoFont = font_load("res/fonts/roboto.ttf", 32);

    stbi_set_flip_vertically_on_load(true);
    blockdata_loadLuaData();
    blockdata_loadArrayTexture();
    world_init(globals.renderRadius);

    threadpool thpool = thpool_init(globals.threads);

    player_init();

    
    bool clicked = false;
    char ft[64], fps[64], pos[64];
    float t, t2 = 0;
    int fpsc = 0;
    while (!glfwWindowShouldClose(window.self)) {
        window_update();
        t += window.dt;
        if (t >= 0.5) {
            sprintf(ft, "Frame Time: %.2f", window.dt * 1000);
            t = 0;
        }

        t2 += window.dt;
        if (t2 >= 1) {
            sprintf(fps, "FPS: %d", fpsc);
            t2 = 0;
            fpsc = 0;
        } else {
            fpsc++;
        }

        sprintf(pos, "Position: %d, %d, %d", (int) camera.position[0], (int) camera.position[1], (int) camera.position[2]);



        //printf("%f %f %f\n", camera.position[0], camera.position[1], camera.position[2]);

        skybox_render(&sky, skyShader);
        camera_use(mainShader);
        world_render(mainShader, thpool);

        shader_setVec3(mainShader, "camera_position", camera.position[0], camera.position[1], camera.position[2]);
        shader_setVec3(mainShader, "camera_direction", camera.front[0], camera.front[1], camera.front[2]);

        shader_setFloat(mainShader, "fog_max", ((world.renderRadius - 1) * CHUNK_SIZE) - CHUNK_SIZE / 2);
        shader_setFloat(mainShader, "fog_min", (world.renderRadius / 2) * CHUNK_SIZE);

        player_update(blockOverlayShader);

        glDisable(GL_DEPTH_TEST);

        sprite2D_render(&uipanel, ALIGN_LEFT, shader2D);
        sprite2D_render(&crosshair, ALIGN_CENTER, shader2D);

        text_render(&robotoFont, textShader, ft, 50.0f, 75.0f, 1.0f, (vec3){0, 0, 0});
        text_render(&robotoFont, textShader, fps, 50.0f, 125.0f, 1.0f, (vec3){0, 0, 0});

        glEnable(GL_DEPTH_TEST);

        glfwSwapBuffers(window.self);
        glfwPollEvents();
    }

    thpool_destroy(thpool);
    window_destroy();

    return 0;
}
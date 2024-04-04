#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "engine/window.h"
#include "engine/shader.h"
#include "engine/camera.h"
#include "engine/player.h"
#include "engine/mesher.h"
#include "engine/sprite2D.h"
#include "engine/skybox.h"
#include "engine/thpool.h"
#include "engine/globals.h"
#include "engine/block_overlay.h"
#include "engine/text.h"

#include "world/world.h"
#include "world/chunk.h"

int main() {
    window_create("samcraft", 1600, 900);
    shader_t mainShader = shader_new("res/shaders/main.vert", "res/shaders/main.frag");
    shader_t blockOverlayShader = shader_new("res/shaders/block_overlay.vert", "res/shaders/block_overlay.frag");
    shader_t shader2D = shader_new("res/shaders/2D.vert", "res/shaders/2D.frag");
    shader_t skyShader = shader_new("res/shaders/sky.vert", "res/shaders/sky.frag");
    shader_t textShader = shader_new("res/shaders/text.vert", "res/shaders/text.frag");

    globals_init();

    shader_use(shader2D);
    mat4 projection2D;
    glm_ortho(0.0f, window.width, window.height, 0.0f, -1.0f, 1.0f, projection2D);
    shader_setMat4(shader2D, "projection", projection2D);

    shader_use(textShader);
    mat4 textProjection;
    glm_ortho(0.0f, window.width, window.height, 0.0f, -1.0f, 1.0f, textProjection);
    shader_setMat4(textShader, "projection", textProjection);

    struct Sprite2D crosshair = sprite2D_new("res/textures/crosshair.png", (ivec2){window.width / 2, window.height / 2}, 32.0f);
    struct Skybox sky = skybox_new();

    struct Font font = font_load("res/textures/pixel-font.png", " ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz01234567890`~!@#$%^&*()-_+=[]{}\\|;:'\"<>,./?�", 32, textShader);
    struct Text testText = text_new("aaa bcdef", (ivec2){100, 100}, font);

    stbi_set_flip_vertically_on_load(true);
    blockdata_loadLuaData();
    blockdata_loadArrayTexture();
    world_init(globals.renderRadius);

    threadpool thpool = thpool_init(globals.threads);

    player_init();
    block_overlay_bind();
    
    bool clicked = false;

    float breakTick = 0;
    int t = 0;
    ivec3 oldPosition;
    while (!glfwWindowShouldClose(window.self)) {
        window_update();
        player_update();

        skybox_render(&sky, skyShader);
        camera_use(mainShader);
        world_render(mainShader, thpool);

        shader_setVec3(mainShader, "camera_position", camera.position[0], camera.position[1], camera.position[2]);
        shader_setVec3(mainShader, "camera_direction", camera.front[0], camera.front[1], camera.front[2]);

        shader_setFloat(mainShader, "fog_max", ((world.renderRadius - 1) * CHUNK_SIZE) - CHUNK_SIZE/2);
        shader_setFloat(mainShader, "fog_min", (world.renderRadius / 2) * CHUNK_SIZE);

        if (window.leftClicked) {
            player_raycast();
            player_destroyBlock(blockOverlayShader);
        } else {
            player.breakTime = 0;
        }

        if (window.rightClicked && !clicked) {
            player_raycast();
            player_placeBlock();
            clicked = true;
        }

        clicked = !window.onMouseRelease;

        text_draw(testText);
        sprite2D_render(&crosshair, shader2D);

        glfwSwapBuffers(window.self);
        glfwPollEvents();
    }

    thpool_destroy(thpool);
    window_destroy();

    return 0;
}
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
#include "engine/text.h"
#include "engine/debugblock.h"

#include "world/world.h"
#include "world/chunk.h"

int main() {
    window_create("samcraft", /*1920, 1080*/ 1600, 900);
    shader_t mainShader = shader_new("res/shaders/main.vert", "res/shaders/main.frag");
    shader_t blockOverlayShader = shader_new("res/shaders/block_overlay.vert", "res/shaders/block_overlay.frag");
    shader_t shader2D = shader_new("res/shaders/2D.vert", "res/shaders/2D.frag");
    shader_t skyShader = shader_new("res/shaders/sky.vert", "res/shaders/sky.frag");
    shader_t textShader = shader_new("res/shaders/text.vert", "res/shaders/text.frag");

    globals_init();
    debugblock_init();

    mat4 projection2D;
    glm_ortho(0.0f, window.width, window.height, 0.0f, -1.0f, 1.0f, projection2D);
    shader_use(shader2D);
    shader_setMat4(shader2D, "projection", projection2D);

    mat4 textProjection;
    glm_ortho(0.0f, window.width, window.height, 0.0f, -1.0f, 1.0f, textProjection);
    shader_use(textShader);
    shader_setMat4(textShader, "projection", textProjection);

    struct Sprite2D crosshair = sprite2D_new("res/textures/crosshair.png", (ivec2){window.width / 2, window.height / 2}, 32.0f);
    struct Skybox sky = skybox_new();

    struct Font robotoFont = font_load("res/fonts/roboto.ttf", 48);

    stbi_set_flip_vertically_on_load(true);
    blockdata_loadLuaData();
    blockdata_loadArrayTexture();
    world_init(globals.renderRadius);

    threadpool thpool = thpool_init(globals.threads);

    player_init();
    
    bool clicked = false;
    while (!glfwWindowShouldClose(window.self)) {
        window_update();

        //printf("%f %f %f\n", camera.position[0], camera.position[1], camera.position[2]);

        skybox_render(&sky, skyShader);
        camera_use(mainShader);
        world_render(mainShader, thpool);

        shader_setVec3(mainShader, "camera_position", camera.position[0], camera.position[1], camera.position[2]);
        shader_setVec3(mainShader, "camera_direction", camera.front[0], camera.front[1], camera.front[2]);

        shader_setFloat(mainShader, "fog_max", ((world.renderRadius - 1) * CHUNK_SIZE) - CHUNK_SIZE / 2);
        shader_setFloat(mainShader, "fog_min", (world.renderRadius / 2) * CHUNK_SIZE);

        player_update(blockOverlayShader);

        sprite2D_render(&crosshair, shader2D);
        text_render(&robotoFont, textShader, "TEST", 25.0f, 25.0f, 1.0f, (vec3){0.5, 0.8f, 0.2f});

        glfwSwapBuffers(window.self);
        glfwPollEvents();
    }

    thpool_destroy(thpool);
    window_destroy();

    return 0;
}
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

#include "world/world.h"
#include "world/chunk.h"

int main() {
    window_create("samcraft", 1600, 900);
    shader_t mainShader = shader_new("res/shaders/main.vert", "res/shaders/main.frag");
    shader_t shader2D = shader_new("res/shaders/2D.vert", "res/shaders/2D.frag");
    shader_t skyShader = shader_new("res/shaders/sky.vert", "res/shaders/sky.frag");

    globals_init();

    shader_use(shader2D);
    mat4 projection2D;
    glUniform1i(glGetUniformLocation(shader2D.ID, "image"), 0);
    glm_ortho(0.0f, window.width, window.height, 0.0f, -1.0f, 1.0f, projection2D);
    shader_setMat4(shader2D, "projection", projection2D);

    struct Sprite2D crosshair = sprite2D_new("res/textures/crosshair.png", (ivec2){window.width / 2, window.height / 2}, 32.0f);
    struct Skybox sky = skybox_new();

    stbi_set_flip_vertically_on_load(true);
    blockdata_loadLuaData();
    blockdata_loadArrayTexture();
    world_init(3);

    threadpool thpool = thpool_init(4);

    player_init();
    
    bool clicked = false;
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

        if (window.leftClicked && !clicked) {
            player_destroyBlock();
            clicked = true;
        }

        if (window.rightClicked && !clicked) {
            player_placeBlock();
            clicked = true;
        }

        clicked = !window.onMouseRelease;

        sprite2D_render(&crosshair, shader2D);

        glfwSwapBuffers(window.self);
        glfwPollEvents();
    }

    thpool_destroy(thpool);
    window_destroy();

    return 0;
}
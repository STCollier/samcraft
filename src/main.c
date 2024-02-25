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
#include "world/world.h"
#include "world/chunk.h"


int main() {
    window_create("samcraft", 1920, 1080);
    shader_t mainShader = shader_new("res/shaders/main.vert", "res/shaders/main.frag");
    shader_t shader2D = shader_new("res/shaders/2D.vert", "res/shaders/2D.frag");

    shader_use(shader2D);
    mat4 projection2D;
    glUniform1i(glGetUniformLocation(shader2D.ID, "image"), 0);
    glm_ortho(0.0f, window.width, window.height, 0.0f, -1.0f, 1.0f, projection2D);
    shader_setMat4(shader2D, "projection", projection2D);

    struct Sprite2D crosshair = sprite2D_new("res/textures/crosshair.png", (ivec2){window.width / 2, window.height / 2}, 16.0f);

    stbi_set_flip_vertically_on_load(true);
    blockdata_loadLuaData();
    blockdata_loadArrayTexture();

    world_init(5);
    player_init();

    bool clicked = false;
    while (!glfwWindowShouldClose(window.self)) {
        window_update();
        player_update();
        shader_use(mainShader);
        camera_use(mainShader);

        world_render(mainShader);

        sprite2D_render(&crosshair, shader2D);

        if (window.leftClicked && !clicked) {
            player_destroyBlock();
            clicked = true;
        }

        if (window.rightClicked && !clicked) {
            player_placeBlock();
            clicked = true;
        }

        clicked = !window.onMouseRelease;

        glfwSwapBuffers(window.self);
        glfwPollEvents();
    }

    window_destroy();

    return 0;
}
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "engine/window.h"
#include "engine/shader.h"
#include "engine/camera.h"
#include "engine/player.h"
#include "engine/mesher.h"
#include "world/world.h"
#include "world/chunk.h"

// Chunk blocks are from 1 through CS_P - 1

int main() {
    window_create("samcraft", 1200, 800); // 1920 1080
    shader_t mainShader = shader_new("res/shaders/main.vert", "res/shaders/main.frag");

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
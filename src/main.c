#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stdio.h>

#include "engine/window.h"
#include "engine/shader.h"
#include "engine/camera.h"
#include "engine/raycast.h"
#include "world/world.h"
#include "world/chunk.h"

// chunks outside of range are trying to be edited

int main() {
    window_create("samcraft", 1000, 800);
    shader_t mainShader = shader_new("res/shaders/main.vert", "res/shaders/main.frag");
    camera_init(60.0f, 50.0f, 0.1f);

    stbi_set_flip_vertically_on_load(true);
    blockdata_loadLuaData();
    blockdata_loadArrayTexture();

    world_init(32);

    struct Ray r = {};

    int blocks[8] = {block_getID("cobblestone"), block_getID("stone"), block_getID("log"), block_getID("planks"), block_getID("sand"), block_getID("dirt"), block_getID("grass"), block_getID("aaaa")};

    int t = 0;
    while (!glfwWindowShouldClose(window.self)) {
        window_update();
        shader_use(mainShader);
        camera_use(mainShader);

        world_render(mainShader);

        if (window.leftClicked) {
            r = ray_cast(camera.position, camera.front);;

            if (r.blockFound) {
                r.chunkToModify->meshSize = 0;

                r.chunkToModify->blocks[blockIndex(r.blockFoundPosition[0], r.blockFoundPosition[1], r.blockFoundPosition[2])].id = BLOCK_AIR;

                world_meshChunk(world_unhashChunk(r.chunkToModify->id));
            }
        }

        if (window.rightClicked) {
            r = ray_cast(camera.position, camera.front);

            if (r.blockFound) {
                r.chunkToModify->meshSize = 0;

                r.chunkToModify->blocks[blockIndex(r.blockFoundPosition[0], r.blockFoundPosition[1] + 1, r.blockFoundPosition[2])].id = blocks[rand() % ARR_SIZE(blocks)];

                world_meshChunk(world_unhashChunk(r.chunkToModify->id));
            }
        }

        //printf("%f %f %f\n", camera.front[0], camera.front[1], camera.front[2]);

        glfwSwapBuffers(window.self);
        glfwPollEvents();
    }
    
    window_destroy();

    return 0;
}
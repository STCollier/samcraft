#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stdio.h>

#include "engine/window.h"
#include "engine/shader.h"
#include "engine/camera.h"
#include "world/chunk.h"

int main() {
    window_create("Window", 1000, 800);
    shader_t mainShader = shader_new("res/shaders/main.vert", "res/shaders/main.frag");
    camera_init(30.0f, 50.0f, 0.1f);

    blockdata_loadLuaData();
    blockdata_loadArrayTexture();

    struct Chunk *chunk = malloc(sizeof(struct Chunk));
    chunk_init(chunk, (ivec2){0, 0});
    chunk_generate(chunk);

    struct Chunk chunkNeighbors[4];
    for (int i = 0; i < 4; i++) chunkNeighbors[i].isNull = true;
    chunk_mesh(chunk, chunkNeighbors);

    while (!glfwWindowShouldClose(window.self)) {
        window_update();
        shader_use(mainShader);
        camera_use(mainShader);

        chunk_bind(chunk);
        chunk_render(chunk, mainShader);

        glfwSwapBuffers(window.self);
        glfwPollEvents();
    }
    
    window_destroy();

    return 0;
}
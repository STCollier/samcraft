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

    float vertices[] = {
        // Positions         // Colors
         32.0f, -32.0f, 0.0f,  1.0f, 0.0f, 0.0f,
        -32.0f, -32.0f, 0.0f,  0.0f, 1.0f, 0.0f,
         0.0f,  32.0f, 0.0f,  0.0f, 0.0f, 1.0f
    };

    struct Chunk *chunk = malloc(sizeof(struct Chunk));
    chunk_init(chunk, (ivec2){0, 0});
    chunk_generate(chunk);

    struct Chunk chunkNeighbors[4];
    for (int i = 0; i < 4; i++) chunkNeighbors[i].isNull = true;
    chunk_mesh(chunk, chunkNeighbors);

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Position Attrib.
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Color Attrib.
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    while (!glfwWindowShouldClose(window.self)) {
        window_update();
        shader_use(mainShader);
        camera_use(mainShader);

        chunk_bind(chunk);
        chunk_render(chunk, mainShader);

        glm_mat4_identity(camera.model); //Reset matrix
        shader_setMat4(mainShader, "model", camera.model);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window.self);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    window_destroy();

    return 0;
}
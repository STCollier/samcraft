#include "debugblock.h"
#include "camera.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

static float vertices[] = {
    0.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f,  1.0f, 0.0f,
    1.0f,  1.0f, 0.0f,
    0.0f,  1.0f, 0.0f,
    0.0f, 0.0f, 0.0f,

    0.0f, 0.0f,  1.0f,
    1.0f, 0.0f,  1.0f,
    1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,
    0.0f,  1.0f,  1.0f,
    0.0f, 0.0f,  1.0f,

    0.0f,  1.0f,  1.0f,
    0.0f,  1.0f, 0.0f,
    0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f,
    0.0f, 0.0f,  1.0f,
    0.0f,  1.0f,  1.0f,

    1.0f,  1.0f,  1.0f,
    1.0f,  1.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f,  1.0f, 
    1.0f,  1.0f,  1.0f,

    0.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f,  1.0f,
    1.0f, 0.0f,  1.0f,
    0.0f, 0.0f,  1.0f,
    0.0f, 0.0f, 0.0f,

    0.0f,  1.0f, 0.0f,
    1.0f,  1.0f, 0.0f,
    1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,
    0.0f,  1.0f,  1.0f,
    0.0f,  1.0f, 0.0f
};

static unsigned int VBO, VAO;

void debugblock_init() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

void debugblock_draw(float x, float y, float z, shader_t shader) {
    glDisable(GL_CULL_FACE);
    shader_use(shader);

    glm_mat4_identity(camera.model);
    glm_translate(camera.model, (vec3){x, y, z});

    shader_setMat4(shader, "model", camera.model);
    shader_setMat4(shader, "projection", camera.projection);
    shader_setMat4(shader, "view", camera.view);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glEnable(GL_CULL_FACE);
}
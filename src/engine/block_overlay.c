#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include "block_overlay.h"
#include "../world/block.h"
#include "camera.h"
#include "player.h"
#include "util.h"

float block_overlay_vertices[] = {
    -0.001f, -0.001f, -0.001f,  -0.001f, -0.001f,
    1.001f, -0.001f, -0.001f,  1.001f, -0.001f,
    1.001f,  1.001f, -0.001f,  1.001f, 1.001f,
    1.001f,  1.001f, -0.001f,  1.001f, 1.001f,
    -0.001f,  1.001f, -0.001f,  -0.001f, 1.001f,
    -0.001f, -0.001f, -0.001f,  -0.001f, -0.001f,

    -0.001f, -0.001f,  1.001f,  -0.001f, -0.001f,
    1.001f, -0.001f,  1.001f,  1.001f, -0.001f,
    1.001f,  1.001f,  1.001f,  1.001f, 1.001f,
    1.001f,  1.001f,  1.001f,  1.001f, 1.001f,
    -0.001f,  1.001f,  1.001f,  -0.001f, 1.001f,
    -0.001f, -0.001f,  1.001f,  -0.001f, -0.001f,

    -0.001f,  1.001f,  1.001f,  1.001f, -0.001f,
    -0.001f,  1.001f, -0.001f,  1.001f, 1.001f,
    -0.001f, -0.001f, -0.001f,  -0.001f, 1.001f,
    -0.001f, -0.001f, -0.001f,  -0.001f, 1.001f,
    -0.001f, -0.001f,  1.001f,  -0.001f, -0.001f,
    -0.001f,  1.001f,  1.001f,  1.001f, -0.001f,

    1.001f,  1.001f,  1.001f,  1.001f, -0.001f,
    1.001f,  1.001f, -0.001f,  1.001f, 1.001f,
    1.001f, -0.001f, -0.001f,  -0.001f, 1.001f,
    1.001f, -0.001f, -0.001f,  -0.001f, 1.001f,
    1.001f, -0.001f,  1.001f,  -0.001f, -0.001f,
    1.001f,  1.001f,  1.001f,  1.001f, -0.001f,

    -0.001f, -0.001f, -0.001f,  -0.001f, 1.001f,
    1.001f, -0.001f, -0.001f,  1.001f, 1.001f,
    1.001f, -0.001f,  1.001f,  1.001f, -0.001f,
    1.001f, -0.001f,  1.001f,  1.001f, -0.001f,
    -0.001f, -0.001f,  1.001f,  -0.001f, -0.001f,
    -0.001f, -0.001f, -0.001f,  -0.001f, 1.001f,

    -0.001f,  1.001f, -0.001f,  -0.001f, 1.001f,
    1.001f,  1.001f, -0.001f,  1.001f, 1.001f,
    1.001f,  1.001f,  1.001f,  1.001f, -0.001f,
    1.001f,  1.001f,  1.001f,  1.001f, -0.001f,
    -0.001f,  1.001f,  1.001f,  -0.001f, -0.001f,
    -0.001f,  1.001f, -0.001f,  -0.001f, 1.001f
};

unsigned int VBO, VAO;

void block_overlay_bind() {
    GL_CHECK(glGenVertexArrays(1, &VAO));
    GL_CHECK(glGenBuffers(1, &VBO));

    GL_CHECK(glBindVertexArray(VAO));

    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, VBO));
    GL_CHECK(glBufferData(GL_ARRAY_BUFFER, sizeof(block_overlay_vertices), block_overlay_vertices, GL_STATIC_DRAW));

    // position attribute
    GL_CHECK(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0));
    GL_CHECK(glEnableVertexAttribArray(0));
    // texture coord attribute
    GL_CHECK(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float))));
    GL_CHECK(glEnableVertexAttribArray(1));
}

void block_overlay_use(shader_t shader, int idx) {
    glDisable(GL_CULL_FACE);

    shader_use(shader);

    GL_CHECK(glActiveTexture(GL_TEXTURE0));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D_ARRAY, block_getBreakArrayTexture()));
    shader_setInt(shader, "breakState", idx);

    shader_setMat4(shader, "projection", camera.projection);
    shader_setMat4(shader, "view", camera.view);
    glm_mat4_identity(camera.model);
    glm_translate(camera.model, (vec3){player.ray.worldPosition[0], player.ray.worldPosition[1], player.ray.worldPosition[2]});
    shader_setMat4(shader, "model", camera.model);

    GL_CHECK(glBindVertexArray(VAO));
    GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, 36));

    GL_CHECK(glBindVertexArray(0));
    glEnable(GL_CULL_FACE);
}
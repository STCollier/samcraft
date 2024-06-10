#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "../core/shader.h"

#include "quad.h"

struct QuadMesh quadmesh_init() {
    struct QuadMesh qm;

    qm.vertices = float_array();
    qm.indices = uint32_t_array();

    return qm;
}

void quad_add(struct QuadMesh* qm, vec2 position, vec2 dimensions, vec2 uv[4], ivec4 color, float scale) {
    size_t offset = qm->vertices.length / 5;
    float rgba = packRGBA(color);

    // Vertices
    float_arr_push(&qm->vertices, position[0]);
    float_arr_push(&qm->vertices, position[1]);
    float_arr_push(&qm->vertices, uv[0][0]);
    float_arr_push(&qm->vertices, uv[0][1]);
    float_arr_push(&qm->vertices, rgba);

    float_arr_push(&qm->vertices, position[0] + dimensions[0]);
    float_arr_push(&qm->vertices, position[1]);
    float_arr_push(&qm->vertices, uv[1][0]);
    float_arr_push(&qm->vertices, uv[1][1]);
    float_arr_push(&qm->vertices, rgba);

    float_arr_push(&qm->vertices, position[0] + dimensions[0]);
    float_arr_push(&qm->vertices, position[1] + dimensions[1]);
    float_arr_push(&qm->vertices, uv[2][0]);
    float_arr_push(&qm->vertices, uv[2][1]);
    float_arr_push(&qm->vertices, rgba);

    float_arr_push(&qm->vertices, position[0]);
    float_arr_push(&qm->vertices, position[1] + dimensions[1]);
    float_arr_push(&qm->vertices, uv[3][0]);
    float_arr_push(&qm->vertices, uv[3][1]);
    float_arr_push(&qm->vertices, rgba);

    // Indices
    uint32_t_arr_push(&qm->indices, offset + 0);
    uint32_t_arr_push(&qm->indices, offset + 3);
    uint32_t_arr_push(&qm->indices, offset + 1);
    uint32_t_arr_push(&qm->indices, offset + 1);
    uint32_t_arr_push(&qm->indices, offset + 3);
    uint32_t_arr_push(&qm->indices, offset + 2);
}

void quadmesh_bind(struct QuadMesh* qm) {
    glGenVertexArrays(1, &qm->VAO);
    glGenBuffers(1, &qm->VBO);
    glGenBuffers(1, &qm->EBO);

    glBindVertexArray(qm->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, qm->VBO);
    glBufferData(GL_ARRAY_BUFFER, qm->vertices.length * sizeof(float), qm->vertices.data, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, qm->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, qm->indices.length * sizeof(uint32_t), qm->indices.data, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0); // pos and tex
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(4 * sizeof(float))); // color 
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void quadmesh_render(struct QuadMesh* qm, shader_t shader, const unsigned int spritesheetTextureID) {
    shader_use(shader);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, spritesheetTextureID);

    glBindVertexArray(qm->VAO);
    glDrawElements(GL_TRIANGLES, qm->indices.length, GL_UNSIGNED_INT, 0);
}

void quadmesh_clear(struct QuadMesh* qm) {
    float_arr_clear(&qm->vertices);
    uint32_t_arr_clear(&qm->indices);
}

void qudmesh_destroy(struct QuadMesh* qm) {
    float_arr_delete(&qm->vertices);
    uint32_t_arr_delete(&qm->indices);

    glDeleteVertexArrays(1, &qm->VAO);
    glDeleteBuffers(1, &qm->VBO);
    glDeleteBuffers(1, &qm->EBO);
}
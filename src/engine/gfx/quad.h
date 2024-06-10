#ifndef QUAD_H
#define QUAD_H

#include "../util/common.h"
#include "../core/shader.h"
#include "image.h"

struct QuadMesh {
    unsigned int VAO, VBO, EBO, textureID;
    arr_float vertices;
    arr_uint32_t indices;
};

struct QuadMesh quadmesh_init();
void quad_add(struct QuadMesh* qm, vec2 position, vec2 dimensions, vec2 uv[4], ivec4 color, float scale);
void quadmesh_bind(struct QuadMesh* qm);
void quadmesh_render(struct QuadMesh* qm, shader_t shader, const unsigned int spritesheetTextureID);
void quadmesh_clear(struct QuadMesh* qm);
void qudmesh_destroy(struct QuadMesh* qm);

#endif
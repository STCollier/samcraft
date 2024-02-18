#include <stb/stb_image.h>
#include "chunk.h"

int blockIndex(int x, int y, int z) {
  return z + (x * CS_P) + (y * CS_P2);
}

void chunk_init(struct Chunk *chunk, ivec3 pos) {
    chunk->isNull = false;

    chunk->position[0] = pos[0];
    chunk->position[1] = pos[1];
    chunk->position[2] = pos[2];

    chunk->voxels = malloc(CS_P3);
    memset(chunk->voxels, 0, CS_P3);

    chunk->light_map = malloc(CS_P3);
    memset(chunk->light_map, 0, CS_P3);
    calculate_light(chunk->voxels, chunk->light_map);

    chunk->vertexList = NULL;
}

void chunk_generate(struct Chunk *chunk) {
    for (int x = 1; x < CS_P - 1; x++) {
        for (int y = 1; y < CS_P - 1; y++) {
            for (int z = 1; z < CS_P - 1; z++) {
                chunk->voxels[blockIndex(x, 25, z)] = 4;
            }
        }
    }

    chunk->voxels[blockIndex(1, 25, 1)] = 1;
    chunk->voxels[blockIndex(50, 25, 1)] = 2;
    chunk->voxels[blockIndex(50, 25, 50)] = 3;
    chunk->voxels[blockIndex(1, 25, 50)] = 5;
}

void chunk_mesh(struct Chunk *chunk, struct Chunk* cn_right, struct Chunk* cn_left, struct Chunk* cn_front, struct Chunk* cn_back, struct Chunk* cn_top, struct Chunk* cn_bottom) {
    for (int x = 0; x < CS_P; x++) {
        for (int y = 0; y < CS_P; y++) {
            for (int z = 0; z < CS_P; z++) {
                if (cn_right != NULL) {
                    chunk->voxels[blockIndex(CS + 1, y, z)] = cn_right->voxels[blockIndex(1, y, z)];
                }
                if (cn_left != NULL) {
                    chunk->voxels[blockIndex(0, y, z)] = cn_left->voxels[blockIndex(CS, y, z)];
                }

                if (cn_back != NULL) {
                    chunk->voxels[blockIndex(x, y, CS + 1)] = cn_back->voxels[blockIndex(x, y, 1)];
                }
                if (cn_front != NULL) {
                    chunk->voxels[blockIndex(x, y, 0)] = cn_front->voxels[blockIndex(x, y, CS)];
                }

                if (cn_top != NULL) {
                    chunk->voxels[blockIndex(x, CS + 1, z)] = cn_top->voxels[blockIndex(x, 1, z)];
                }
                if (cn_bottom != NULL) {
                    chunk->voxels[blockIndex(x, 0, z)] = cn_bottom->voxels[blockIndex(x, CS, z)];
                }
            }
        }
    }

    chunk->vertexList = mesh(chunk->voxels, chunk->light_map);
}

void chunk_bind(struct Chunk *chunk) {
    glGenVertexArrays(1, &chunk->VAO);
    glGenBuffers(1, &chunk->VBO);

    glBindBuffer(GL_ARRAY_BUFFER, chunk->VBO);
    glBindVertexArray(chunk->VAO);
    glEnableVertexAttribArray(0);
    glVertexAttribIPointer(0, sizeof(uint32_t), GL_UNSIGNED_INT, sizeof(uint32_t), (void*)0);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(chunk->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, chunk->VBO);
    glBufferData(GL_ARRAY_BUFFER, chunk->vertexList->size * sizeof(uint32_t), chunk->vertexList->data, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void chunk_render(struct Chunk *chunk, shader_t shader) {
    glm_mat4_identity(camera.model);
    glm_translate(camera.model, (vec3){chunk->position[0] * CHUNK_SIZE, chunk->position[1] * CHUNK_SIZE, chunk->position[2] * CHUNK_SIZE});
    shader_setMat4(shader, "model", camera.model);

    glBindVertexArray(chunk->VAO);
    glDrawArrays(GL_TRIANGLES, 0, chunk->vertexList->size);
    glBindVertexArray(0);
}
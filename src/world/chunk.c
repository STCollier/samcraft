#include <stb/stb_image.h>
#include "chunk.h"
#include "worldgen.h"

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

    chunk->vertexList = malloc(sizeof(struct VertexList));
}

void chunk_generate(struct Chunk *chunk) {

    for (int x = 1; x < CS_P - 1; x++) {
        for (int z = 1; z < CS_P - 1; z++) {

            int height = noiseHeight((ivec2){x, z}, (ivec2){chunk->position[0] * CHUNK_SIZE, -(chunk->position[2] * CHUNK_SIZE)});

            for (int y = 1; y < CS_P - 1; y++) {
                int chunkHeight = (chunk->position[1] * CHUNK_SIZE) + y;

                if (chunkHeight <= height) {
                    chunk->voxels[blockIndex(x, y, z)] = chunkHeight == height ? block_getID("grass") : block_getID("dirt");
                } else if (chunk->voxels[blockIndex(x, y, z)] == BLOCK_AIR && chunkHeight < WATER_HEIGHT) {
                    chunk->voxels[blockIndex(x, y, z)] = block_getID("water");
                } else {
                    chunk->voxels[blockIndex(x, y, z)] = BLOCK_AIR;
                }

                if ((chunk->voxels[blockIndex(x, y, z)] == block_getID("grass") || chunk->voxels[blockIndex(x, y, z)] == block_getID("dirt")) && chunkHeight > 0 && chunkHeight < WATER_HEIGHT + 3) {
                    chunk->voxels[blockIndex(x, y, z)] = block_getID("sand");
                }
            }
        }
    }
}

void chunk_mesh(struct Chunk *chunk, struct Chunk* cn_right, struct Chunk* cn_left, struct Chunk* cn_front, struct Chunk* cn_back, struct Chunk* cn_top, struct Chunk* cn_bottom) {
    uint8_t* opaque = malloc(CS_P3);
    memset(opaque, 0, CS_P3);

    uint8_t* transparent = malloc(CS_P3);
    memset(transparent, 0, CS_P3);

    uint8_t water = block_getID("water");
        for (int x = 0; x < CS_P; x++) {
            for (int y = 0; y < CS_P; y++) {
                for (int z = 0; z < CS_P; z++) {
                    // Chunk Neighbors
                    if (x == CS + 1)   chunk->voxels[blockIndex(CS + 1, y, z)] = cn_right->voxels[blockIndex(1, y, z)];
                    if (x == 0)    chunk->voxels[blockIndex(0, y, z)] = cn_left->voxels[blockIndex(CS, y, z)];
                    if (z == CS + 1)    chunk->voxels[blockIndex(x, y, CS + 1)] = cn_back->voxels[blockIndex(x, y, 1)];
                    if (z == 0)   chunk->voxels[blockIndex(x, y, 0)] = cn_front->voxels[blockIndex(x, y, CS)];
                    if (y == CS + 1)     chunk->voxels[blockIndex(x, CS + 1, z)] = cn_top->voxels[blockIndex(x, 1, z)];
                    if (y == 0)  chunk->voxels[blockIndex(x, 0, z)] = cn_bottom->voxels[blockIndex(x, CS, z)];
                    
                    int idx = blockIndex(x, y, z);

                    // Sort opaque and transparent blocks into separate arrays to mesh
                    if (chunk->voxels[idx] == water) {
                        transparent[idx] = chunk->voxels[idx];
                    } else {
                        opaque[idx] = chunk->voxels[idx];
                    }
                }
            }
        }

        chunk->vertexList->opaque = mesh(chunk->voxels, true);
        chunk->vertexList->transparent = mesh(transparent, false);

    free(opaque);
    free(transparent);
}

void chunk_bind(struct Chunk *chunk) {
    GL_CHECK(glGenVertexArrays(1, &chunk->VAO));
    GL_CHECK(glGenBuffers(1, &chunk->VBO));

    GL_CHECK(glBindVertexArray(chunk->VAO));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, chunk->VBO));
    GL_CHECK(glBufferData(GL_ARRAY_BUFFER, chunk->vertexList->opaque->size * sizeof(vertex_t), chunk->vertexList->opaque->data, GL_STATIC_DRAW));
    
    GL_CHECK(glVertexAttribIPointer(0, 4, GL_UNSIGNED_INT, sizeof(vertex_t), (void*)0));
    glEnableVertexAttribArray(0);

    GL_CHECK(glVertexAttribIPointer(1, 2, GL_UNSIGNED_SHORT, sizeof(vertex_t), (void*)offsetof(vertex_t, u_v)));
    glEnableVertexAttribArray(1);

    GL_CHECK(glVertexAttribIPointer(2, 3, GL_UNSIGNED_BYTE, sizeof(vertex_t), (void*)offsetof(vertex_t, norm_ao)));
    glEnableVertexAttribArray(2);

    GL_CHECK(glGenVertexArrays(1, &chunk->tVAO));
    GL_CHECK(glGenBuffers(1, &chunk->tVBO));

    GL_CHECK(glBindVertexArray(chunk->tVAO));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, chunk->tVBO));
    GL_CHECK(glBufferData(GL_ARRAY_BUFFER, chunk->vertexList->transparent->size * sizeof(vertex_t), chunk->vertexList->transparent->data, GL_STATIC_DRAW));
    
    GL_CHECK(glVertexAttribIPointer(0, 4, GL_UNSIGNED_INT, sizeof(vertex_t), (void*)0));
    glEnableVertexAttribArray(0);

    GL_CHECK(glVertexAttribIPointer(1, 2, GL_UNSIGNED_SHORT, sizeof(vertex_t), (void*)offsetof(vertex_t, u_v)));
    glEnableVertexAttribArray(1);

    GL_CHECK(glVertexAttribIPointer(2, 3, GL_UNSIGNED_BYTE, sizeof(vertex_t), (void*)offsetof(vertex_t, norm_ao)));
    glEnableVertexAttribArray(2);
}

void chunk_render(struct Chunk *chunk, shader_t shader, bool pass) {
    glm_mat4_identity(camera.model);
    glm_translate(camera.model, (vec3){chunk->position[0] * (CHUNK_SIZE), chunk->position[1] * (CHUNK_SIZE), chunk->position[2] * (CHUNK_SIZE)});
    shader_setMat4(shader, "model", camera.model);

        GL_CHECK(glBindVertexArray(chunk->VAO));
        GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, chunk->vertexList->opaque->size));

    if (pass) {
        GL_CHECK(glBindVertexArray(chunk->VAO));
        GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, chunk->vertexList->opaque->size));
    } else {
        glm_mat4_identity(camera.model);
        glm_translate(camera.model, (vec3){chunk->position[0] * (CHUNK_SIZE), chunk->position[1] * (CHUNK_SIZE) - 0.25, chunk->position[2] * (CHUNK_SIZE)});
        shader_setMat4(shader, "model", camera.model);
    
        GL_CHECK(glBindVertexArray(chunk->tVAO));
        GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, chunk->vertexList->transparent->size));
    }
}
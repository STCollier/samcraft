#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "stb/stb_image.h"
#include "uthash/uthash.h"

#include "../engine/core/shader.h"
#include "../engine/core/camera.h"
#include "../engine/util/util.h"
#include "../engine/util/types.h"
#include "../engine/func/mesher.h"
#include "../engine/gfx/light.h"

#include "worldgen.h"
#include "block.h"
#include "chunk.h"

int blockIndex(int x, int y, int z) {
  return z + (x * CS_P) + (y * CS_P2);
}

void chunk_init(struct Chunk *chunk, ivec3 pos) {
    chunk->position[0] = pos[0];
    chunk->position[1] = pos[1];
    chunk->position[2] = pos[2];

    chunk->voxels = malloc(CS_P3);
    memset(chunk->voxels, 0, CS_P3);

    chunk->empty = false;
    chunk->state = ADDED;
    chunk->addedToMeshQueue = false;
}

void chunk_generate(struct Chunk *chunk) {
    uint8_t grass = block_getID("grass");
    uint8_t dirt = block_getID("dirt");
    uint8_t sand = block_getID("sand");
    uint8_t water = block_getID("water");

    int b = 0;
    for (int x = 0; x < CS_P; x++) {
        for (int y = 0; y < CS_P; y++) {
            for (int z = 0; z < CS_P; z++) {
                int height = noiseHeight((ivec3){x, y, z}, (ivec3){(chunk->position[0] * CHUNK_SIZE), -(chunk->position[1] * CHUNK_SIZE), -(chunk->position[2] * CHUNK_SIZE)});

                int chunkHeight = (chunk->position[1] * CHUNK_SIZE) + y;

                if (chunkHeight <= height) {
                    chunk->voxels[blockIndex(x, y, z)] = dirt;
                } else {
                    chunk->voxels[blockIndex(x, y, z)] = BLOCK_AIR;
                }

                b += chunk->voxels[blockIndex(x, y, z)];
            }
        }
    }

    chunk->empty = !b;

    //printf("Chunk Empty: [%d %d %d]: %s\n", chunk->position[0], chunk->position[1], chunk->position[2], chunk->empty ? "true" : "false");

    for (int x = 0; x < CS_P; x++) {
        for (int y = 0; y < CS_P; y++) {
            for (int z = 0; z < CS_P; z++) {
                if (y + 1 < CS_P) {
                    if (chunk->voxels[blockIndex(x, y + 1, z)] == BLOCK_AIR && chunk->voxels[blockIndex(x, y, z)] == dirt) {
                        chunk->voxels[blockIndex(x, y, z)] = grass;
                    }
                }
            }
        }
    }

    chunk->state = GENERATED;
}

void chunk_mesh(struct Chunk *chunk) {
    uint8_t* opaque = malloc(CS_P3);
    memset(opaque, 0, CS_P3);

    uint8_t* transparent = malloc(CS_P3);
    memset(transparent, 0, CS_P3);

    uint8_t water = block_getID("water");

    for (int x = 0; x < CS_P; x++) {
        for (int y = 0; y < CS_P; y++) {
            for (int z = 0; z < CS_P; z++) {
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

    chunk->mesh.opaque = mesh(opaque, true);
    chunk->mesh.transparent = mesh(transparent, false);

    free(opaque);
    free(transparent);

    chunk->state = MESHED;
}

void chunk_remesh(struct Chunk *chunk, struct Chunk* cn_right, struct Chunk* cn_left, struct Chunk* cn_front, struct Chunk* cn_back, struct Chunk* cn_top, struct Chunk* cn_bottom) {
    uint8_t* opaque = malloc(CS_P3);
    memset(opaque, 0, CS_P3);

    uint8_t* transparent = malloc(CS_P3);
    memset(transparent, 0, CS_P3);

    uint8_t water = block_getID("water");

    for (int x = 0; x < CS_P; x++) {
        for (int y = 0; y < CS_P; y++) {
            for (int z = 0; z < CS_P; z++) {
                int idx = blockIndex(x, y, z);
                if (x == CS + 1)   chunk->voxels[blockIndex(CS + 1, y, z)] = cn_right->voxels[blockIndex(1, y, z)];
                if (x == 0)    chunk->voxels[blockIndex(0, y, z)] = cn_left->voxels[blockIndex(CS, y, z)];
                if (z == CS + 1)    chunk->voxels[blockIndex(x, y, CS + 1)] = cn_back->voxels[blockIndex(x, y, 1)];
                if (z == 0)   chunk->voxels[blockIndex(x, y, 0)] = cn_front->voxels[blockIndex(x, y, CS)];
                if (y == CS + 1)     chunk->voxels[blockIndex(x, CS + 1, z)] = cn_top->voxels[blockIndex(x, 1, z)];
                if (y == 0)  chunk->voxels[blockIndex(x, 0, z)] = cn_bottom->voxels[blockIndex(x, CS, z)];
                // Sort opaque and transparent blocks into separate arrays to mesh
                if (chunk->voxels[idx] == water) {
                    transparent[idx] = chunk->voxels[idx];
                } else {
                    opaque[idx] = chunk->voxels[idx];
                }
            }
        }
    }

    chunk->mesh.opaque = mesh(opaque, true);
    chunk->mesh.transparent = mesh(transparent, false);

    free(opaque);
    free(transparent);

    chunk->state = MESHED;
}

void chunk_bind(struct Chunk *chunk) {

    for (int i = 0; i < 6; i++) {
        glGenVertexArrays(1, &chunk->VAO[i]);
        glGenBuffers(1, &chunk->VBO[i]);
        glGenBuffers(1, &chunk->EBO[i]);

        glBindVertexArray(chunk->VAO[i]);
        glBindBuffer(GL_ARRAY_BUFFER, chunk->VBO[i]);
        glBufferData(GL_ARRAY_BUFFER, chunk->mesh.opaque.meshes[i].vertices.length * sizeof(uint64_t), chunk->mesh.opaque.meshes[i].vertices.data, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk->EBO[i]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, chunk->mesh.opaque.meshes[i].indices.length * sizeof(uint32_t), chunk->mesh.opaque.meshes[i].indices.data, GL_STATIC_DRAW);
        
        glVertexAttribIPointer(0, 2, GL_UNSIGNED_INT, 0, (void*)0);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        /*glGenVertexArrays(1, &chunk->tVAO[i]);
        glGenBuffers(1, &chunk->tVBO[i]);
        glGenBuffers(1, &chunk->tEBO[i]);

        glBindVertexArray(chunk->tVAO[i]);
        glBindBuffer(GL_ARRAY_BUFFER, chunk->tVBO[i]);
        glBufferData(GL_ARRAY_BUFFER, chunk->mesh.transparent.meshes[i].vertices.length * sizeof(uint64_t), chunk->mesh.transparent.meshes[i].vertices.data, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk->tEBO[i]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, chunk->mesh.transparent.meshes[i].indices.length * sizeof(uint32_t), chunk->mesh.transparent.meshes[i].indices.data, GL_STATIC_DRAW);
        
        glVertexAttribIPointer(0, 2, GL_UNSIGNED_INT, 0, (void*)0);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);*/
        }

        chunk->state = BOUND;
    }

void chunk_render(struct Chunk *chunk, shader_t shader, bool draw[6], bool pass) {
    shader_use(shader);
    shader_setInt(shader, "textureArray", 0);
    shader_setInt(shader, "normalArray", 1);
    shader_setInt(shader, "shadowMap", 2);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, block_getDiffuseArrayTexture());

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D_ARRAY, block_getNormalArrayTexture());

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, light.depthMap.map);

    vec3 chunkTranslation;
    glm_vec3_copy((vec3){
        chunk->position[0] * (CHUNK_SIZE),
        chunk->position[1] * (CHUNK_SIZE), 
        chunk->position[2] * (CHUNK_SIZE)
    }, chunkTranslation);

    shader_setVec3(shader, "chunk_translation", chunkTranslation[0], chunkTranslation[1], chunkTranslation[2]);

    for (int i = 0; i < 6; i++) {
        bool toRender = draw[i];

        if (toRender) {
            glBindVertexArray(chunk->VAO[i]);
            glDrawElements(GL_TRIANGLES, chunk->mesh.opaque.meshes[i].indices.length, GL_UNSIGNED_INT, 0);
        }
    }
}
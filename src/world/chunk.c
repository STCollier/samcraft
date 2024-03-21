#include <stb/stb_image.h>
#include "chunk.h"
#include "worldgen.h"

int blockIndex(int x, int y, int z) {
  return z + (x * CS_P) + (y * CS_P2);
}

void chunk_init(struct Chunk *chunk, ivec3 pos) {
    chunk->position[0] = pos[0];
    chunk->position[1] = pos[1];
    chunk->position[2] = pos[2];

    chunk->voxels = malloc(CS_P3);
    memset(chunk->voxels, 0, CS_P3);

    chunk->mesh = malloc(sizeof(struct ChunkMesh));

    chunk->state = ADDED;
    chunk->addedToMeshQueue = false;
}

void chunk_generate(struct Chunk *chunk) {
    uint8_t grass = block_getID("grass");
    uint8_t dirt = block_getID("dirt");
    uint8_t sand = block_getID("sand");
    uint8_t water = block_getID("water");

    for (int x = 0; x < CS_P; x++) {
        for (int z = 0; z < CS_P; z++) {

            int height = noiseHeight((ivec2){x, z}, (ivec2){chunk->position[0] * CHUNK_SIZE, -(chunk->position[2] * CHUNK_SIZE)});

            for (int y = 0; y < CS_P; y++) {
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

    chunk->mesh->opaque = mesh(opaque, true);
    chunk->mesh->transparent = mesh(transparent, false);

    free(opaque);
    free(transparent);

    puts("IM MESHING");

    chunk->state = MESHED;
}

void chunk_bind(struct Chunk *chunk) {
    GL_CHECK(glGenVertexArrays(1, &chunk->VAO));
    GL_CHECK(glGenBuffers(1, &chunk->VBO));
    GL_CHECK(glGenBuffers(1, &chunk->EBO));

    GL_CHECK(glBindVertexArray(chunk->VAO));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, chunk->VBO));
    GL_CHECK(glBufferData(GL_ARRAY_BUFFER, chunk->mesh->opaque->vertices->size * sizeof(uint64_t), chunk->mesh->opaque->vertices->data, GL_STATIC_DRAW));

    GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk->EBO));
    GL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, chunk->mesh->opaque->indices->size * sizeof(uint32_t), chunk->mesh->opaque->indices->data, GL_STATIC_DRAW));
    
    GL_CHECK(glVertexAttribIPointer(0, 2, GL_UNSIGNED_INT, 0, (void*)0));
    GL_CHECK(glEnableVertexAttribArray(0));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));
    GL_CHECK(glBindVertexArray(0));

    GL_CHECK(glGenVertexArrays(1, &chunk->tVAO));
    GL_CHECK(glGenBuffers(1, &chunk->tVBO));
    GL_CHECK(glGenBuffers(1, &chunk->tEBO));

    GL_CHECK(glBindVertexArray(chunk->tVAO));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, chunk->tVBO));
    GL_CHECK(glBufferData(GL_ARRAY_BUFFER, chunk->mesh->transparent->vertices->size * sizeof(uint64_t), chunk->mesh->transparent->vertices->data, GL_STATIC_DRAW));

    GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk->tEBO));
    GL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, chunk->mesh->transparent->indices->size * sizeof(uint32_t), chunk->mesh->transparent->indices->data, GL_STATIC_DRAW));
    
    GL_CHECK(glVertexAttribIPointer(0, 2, GL_UNSIGNED_INT, 0, (void*)0));
    GL_CHECK(glEnableVertexAttribArray(0));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));
    GL_CHECK(glBindVertexArray(0));

    chunk->state = BOUND;
}

void chunk_render(struct Chunk *chunk, shader_t shader, bool pass) {
    glm_mat4_identity(camera.model);
    glm_translate(camera.model, (vec3){chunk->position[0] * (CHUNK_SIZE), chunk->position[1] * (CHUNK_SIZE), chunk->position[2] * (CHUNK_SIZE)});
    shader_setMat4(shader, "model", camera.model);

    GL_CHECK(glBindVertexArray(chunk->VAO));
    GL_CHECK(glDrawElements(GL_TRIANGLES, chunk->mesh->opaque->indices->size, GL_UNSIGNED_INT, 0));

    if (pass) {
        GL_CHECK(glBindVertexArray(chunk->VAO));
        GL_CHECK(glDrawElements(GL_TRIANGLES, chunk->mesh->opaque->indices->size, GL_UNSIGNED_INT, 0));
    } else {
        glm_mat4_identity(camera.model);
        glm_translate(camera.model, (vec3){chunk->position[0] * (CHUNK_SIZE), chunk->position[1] * (CHUNK_SIZE) - 0.25, chunk->position[2] * (CHUNK_SIZE)});
        shader_setMat4(shader, "model", camera.model);

        GL_CHECK(glBindVertexArray(chunk->tVAO));
        GL_CHECK(glDrawElements(GL_TRIANGLES, chunk->mesh->transparent->indices->size, GL_UNSIGNED_INT, 0));
    }
}
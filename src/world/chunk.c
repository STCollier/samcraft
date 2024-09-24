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
#include "world.h"

static arr_light_node_t lightBFSQueue;
static arr_light_removal_node_t lightRemovalBFSQueue;

void chunkmanager_init() {
    lightBFSQueue = light_node_t_array();
    lightRemovalBFSQueue = light_removal_node_t_array();
}

// CS_P = CHUNK_SIZE + 2
// CS_P2 = CS_P * CS_P
inline int blockIndex(int x, int y, int z) {
    return z + (x * CS_P) + (y * CS_P2);
}

struct Chunk* getNeighboringChunk(struct Chunk* chunk, Direction direction) {
    switch (direction) {
        case RIGHT:
            return world_getChunk((ivec3){chunk->position[0] + 1, chunk->position[1], chunk->position[2]});
        case LEFT:
            return world_getChunk((ivec3){chunk->position[0] - 1, chunk->position[1], chunk->position[2]});
        case TOP:
            return world_getChunk((ivec3){chunk->position[0], chunk->position[1] + 1, chunk->position[2]});
        case BOTTOM:
            return world_getChunk((ivec3){chunk->position[0], chunk->position[1] - 1, chunk->position[2]});
        case FRONT:
            return world_getChunk((ivec3){chunk->position[0], chunk->position[1], chunk->position[2] + 1});
        case BACK:
            return world_getChunk((ivec3){chunk->position[0], chunk->position[1], chunk->position[2] - 1});
    }

    ERROR("No neighboring chunk found in function!");
    exit(EXIT_FAILURE);
}

int chunk_getTorchlight(struct Chunk* chunk, ivec3 position) {
    return chunk->lightMap.torchlightMap[blockIndex(position[1], position[2], position[0])];
}

void chunk_setTorchlight(struct Chunk* chunk, ivec3 position, int val) {
    chunk->lightMap.torchlightMap[blockIndex(position[1], position[2], position[0])] = val;
}

int chunk_getRedLight(struct Chunk* chunk, ivec3 position) {
    return (chunk->lightMap.torchlightMap[blockIndex(position[0], position[1], position[2])] >> 8) & 0xF;
}

void chunk_setRedLight(struct Chunk* chunk, ivec3 position, int val) {
    chunk->lightMap.torchlightMap[blockIndex(position[0], position[1], position[2])] = (chunk->lightMap.torchlightMap[blockIndex(position[0], position[1], position[2])] & 0xF0FF) | (val << 8);
}

int chunk_getGreenLight(struct Chunk* chunk, ivec3 position) {
    return (chunk->lightMap.torchlightMap[blockIndex(position[0], position[1], position[2])] >> 4) & 0xF;
}

void chunk_setGreenLight(struct Chunk* chunk, ivec3 position, int val) {
    chunk->lightMap.torchlightMap[blockIndex(position[0], position[1], position[2])] = (chunk->lightMap.torchlightMap[blockIndex(position[0], position[1], position[2])] & 0xFF0F) | (val << 4);
}

int chunk_getBlueLight(struct Chunk* chunk, ivec3 position) {
    return chunk->lightMap.torchlightMap[blockIndex(position[0], position[1], position[2])] & 0xF;
}

void chunk_setBlueLight(struct Chunk* chunk, ivec3 position, int val) {
    chunk->lightMap.torchlightMap[blockIndex(position[0], position[1], position[2])] = (chunk->lightMap.torchlightMap[blockIndex(position[0], position[1], position[2])] & 0xFFF0) | (val);
}

void chunk_updateLightMap(struct Chunk* chunk) {
    glBindTexture(GL_TEXTURE_3D, chunk->lightMap.texture);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_R16UI, CS_P, CS_P, CS_P, 0, GL_RED_INTEGER, GL_UNSIGNED_SHORT, chunk->lightMap.torchlightMap);
    //glGenerateMipmap(GL_TEXTURE_3D);
}

// Chunk lightmap has dimensions of (CHUNK_SIZE + 2)^3 or CS_P^3 
void chunk_addLight(struct Chunk* chunk, ivec3 p, int level) {
    ivec3 position = {p[0] - 1, p[1] - 1, p[2] - 1};
    chunk_setTorchlight(chunk, position, level);

    light_node_t_arr_push(&lightBFSQueue, (light_node_t) {
        .chunk = chunk,
        .position = {chunk->position[0]*CHUNK_SIZE + position[0], chunk->position[1]*CHUNK_SIZE + position[1], chunk->position[2]*CHUNK_SIZE + position[2]} // World Space
    });

    while (lightBFSQueue.length > 0) {
        struct LightNode node = light_node_t_arr_pop(&lightBFSQueue);

        struct Chunk* c = node.chunk;

        // World Space Position
        int x = node.position[0];
        int y = node.position[1];
        int z = node.position[2];

        int lightLevel = chunk_getTorchlight(c, (ivec3){(x % CHUNK_SIZE), (y % CHUNK_SIZE), (z % CHUNK_SIZE)});

        // World Space
        ivec3 directions[6] = {
            {x + 1, y, z},
            {x - 1, y, z},
            {x, y + 1, z},
            {x, y - 1, z},
            {x, y, z + 1},
            {x, y, z - 1}
        };

        ivec3 propagationPosition; // World Space
        for (int i = 0; i < 6; i++) {
            glm_ivec3_copy(directions[i], propagationPosition);

            ivec3 chunkSpace = (ivec3){(propagationPosition[0] % CHUNK_SIZE), (propagationPosition[1] % CHUNK_SIZE), (propagationPosition[2] % CHUNK_SIZE)};

            // Get chunk based on world space position
            c = world_getChunk((ivec3){
                propagationPosition[0] >= 0 ? (propagationPosition[0] / CHUNK_SIZE) : (propagationPosition[0] / CHUNK_SIZE) - 1, 
                propagationPosition[1] >= 0 ? (propagationPosition[1] / CHUNK_SIZE) : (propagationPosition[1] / CHUNK_SIZE) - 1,
                propagationPosition[2] >= 0 ? (propagationPosition[2] / CHUNK_SIZE) : (propagationPosition[2] / CHUNK_SIZE) - 1
            });


            int p = blockIndex((propagationPosition[0] + 1) % CHUNK_SIZE, (propagationPosition[1] + 1) % CHUNK_SIZE, (propagationPosition[2] + 1) % CHUNK_SIZE);
            if (c->voxels[p] == BLOCK_AIR && chunk_getTorchlight(c, chunkSpace) + 2 <= lightLevel) { 
                chunk_setTorchlight(c, chunkSpace, lightLevel - 1);

                light_node_t_arr_push(&lightBFSQueue, (light_node_t) {
                    .chunk = c,
                    .position = {propagationPosition[0], propagationPosition[1], propagationPosition[2]} // World Space
                });
            }
        }
    }

    struct Chunk* cc[10] = {
        world_getChunk((ivec3){chunk->position[0] + 1, chunk->position[1], chunk->position[2]        }),
        world_getChunk((ivec3){chunk->position[0] - 1, chunk->position[1], chunk->position[2]        }),
        world_getChunk((ivec3){chunk->position[0]    , chunk->position[1] + 1, chunk->position[2]    }),
        world_getChunk((ivec3){chunk->position[0]    , chunk->position[1] - 1, chunk->position[2]    }),
        world_getChunk((ivec3){chunk->position[0]    , chunk->position[1]    , chunk->position[2] + 1}),
        world_getChunk((ivec3){chunk->position[0]    , chunk->position[1]    , chunk->position[2] - 1}),

        world_getChunk((ivec3){chunk->position[0] - 1, chunk->position[1]    , chunk->position[2] - 1}),
        world_getChunk((ivec3){chunk->position[0] - 1, chunk->position[1]    , chunk->position[2] + 1}),
        world_getChunk((ivec3){chunk->position[0] + 1, chunk->position[1]    , chunk->position[2] - 1}),
        world_getChunk((ivec3){chunk->position[0] + 1, chunk->position[1]    , chunk->position[2] + 1})
    };

    chunk_updateLightMap(chunk);
    for (int i = 0; i < 10; i++) chunk_updateLightMap(cc[i]);
}

void chunk_removeLight(struct Chunk* chunk, ivec3 p) {
    ivec3 position = {p[0] - 1, p[1] - 1, p[2] - 1};

    light_removal_node_t_arr_push(&lightRemovalBFSQueue, (light_removal_node_t) {
        .chunk = chunk,
        .position = {chunk->position[0]*CHUNK_SIZE + position[0], chunk->position[1]*CHUNK_SIZE + position[1], chunk->position[2]*CHUNK_SIZE + position[2]}, // World Space
        .value = chunk_getTorchlight(chunk, position)
    });

    chunk_setTorchlight(chunk, position, 0);

    while (lightRemovalBFSQueue.length > 0) {
        struct LightRemovalNode node = light_removal_node_t_arr_pop(&lightRemovalBFSQueue);

        struct Chunk* c = node.chunk;

        int lightLevel = node.value;

        // World Space Position
        int x = node.position[0];
        int y = node.position[1];
        int z = node.position[2];

        // World Space
        ivec3 directions[6] = {
            {x + 1, y, z},
            {x - 1, y, z},
            {x, y + 1, z},
            {x, y - 1, z},
            {x, y, z + 1},
            {x, y, z - 1}
        };

        ivec3 propagationPosition; // World Space
        for (int i = 0; i < 6; i++) {
            glm_ivec3_copy(directions[i], propagationPosition);

            ivec3 chunkSpace = (ivec3){(propagationPosition[0] % CHUNK_SIZE), (propagationPosition[1] % CHUNK_SIZE), (propagationPosition[2] % CHUNK_SIZE)};

            // Get chunk based on world space position
            c = world_getChunk((ivec3){
                propagationPosition[0] >= 0 ? (propagationPosition[0] / CHUNK_SIZE) : (propagationPosition[0] / CHUNK_SIZE) - 1, 
                propagationPosition[1] >= 0 ? (propagationPosition[1] / CHUNK_SIZE) : (propagationPosition[1] / CHUNK_SIZE) - 1,
                propagationPosition[2] >= 0 ? (propagationPosition[2] / CHUNK_SIZE) : (propagationPosition[2] / CHUNK_SIZE) - 1
            });

            int neighborLevel = chunk_getTorchlight(c, chunkSpace);

            if (neighborLevel != 0 && neighborLevel < lightLevel) {
                chunk_setTorchlight(c, chunkSpace, 0);

                light_removal_node_t_arr_push(&lightRemovalBFSQueue, (light_removal_node_t) {
                    .chunk = c,
                    .position = {propagationPosition[0], propagationPosition[1], propagationPosition[2]}, // World Space
                    .value = neighborLevel
                });
            } else if (neighborLevel <= lightLevel) {
                light_node_t_arr_push(&lightBFSQueue, (light_node_t) {
                    .chunk = c,
                    .position = {propagationPosition[0], propagationPosition[1], propagationPosition[2]} // World Space
                });
            }
        }
    }
}

void chunk_init(struct Chunk *chunk, ivec3 pos) {
    chunk->position[0] = pos[0];
    chunk->position[1] = pos[1];
    chunk->position[2] = pos[2];

    chunk->voxels = malloc(CS_P3);
    memset(chunk->voxels, 0, CS_P3);

    chunk->lightMap.torchlightMap = malloc(CS_P3 * sizeof(uint16_t));
    for (int i = 0; i < CS_P3; i++) chunk->lightMap.torchlightMap[i] = 1;
    
    glGenTextures(1, &chunk->lightMap.texture);
    glBindTexture(GL_TEXTURE_3D, chunk->lightMap.texture);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage3D(GL_TEXTURE_3D, 0, GL_R16UI, CS_P, CS_P, CS_P, 0, GL_RED_INTEGER, GL_UNSIGNED_SHORT, chunk->lightMap.torchlightMap);
    //glGenerateMipmap(GL_TEXTURE_3D);

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
    uint8_t grass = block_getID("grass");

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

void chunk_remesh(struct Chunk *chunk, struct Chunk* cn_right, struct Chunk* cn_left, struct Chunk* cn_top, struct Chunk* cn_bottom, struct Chunk* cn_front, struct Chunk* cn_back) {
    uint8_t* opaque = malloc(CS_P3);
    memset(opaque, 0, CS_P3);

    uint8_t* transparent = malloc(CS_P3);
    memset(transparent, 0, CS_P3);

    uint8_t water = block_getID("water");
    uint8_t dirt = block_getID("dirt");

    for (int x = 0; x < CS_P; x++) {
        for (int y = 0; y < CS_P; y++) {
            for (int z = 0; z < CS_P; z++) {
                int idx = blockIndex(x, y, z);
                if (x == CS + 1)   chunk->voxels[blockIndex(CS + 1, y, z)] = cn_right->voxels[blockIndex(1, y, z)];
                if (x == 0)    chunk->voxels[blockIndex(0, y, z)] = cn_left->voxels[blockIndex(CS, y, z)];
                if (y == CS + 1)     chunk->voxels[blockIndex(x, CS + 1, z)] = cn_top->voxels[blockIndex(x, 1, z)];
                if (y == 0)  chunk->voxels[blockIndex(x, 0, z)] = cn_bottom->voxels[blockIndex(x, CS, z)];
                if (z == CS + 1)    chunk->voxels[blockIndex(x, y, CS + 1)] = cn_front->voxels[blockIndex(x, y, 1)];
                if (z == 0)   chunk->voxels[blockIndex(x, y, 0)] = cn_back->voxels[blockIndex(x, y, CS)];

                if (block_getLightLevel(chunk->voxels[idx])) chunk_addLight(chunk, (ivec3){x, y, z}, 15);
                if (chunk->voxels[idx] == dirt) chunk_removeLight(chunk, (ivec3){x, y, z});

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
    }

    chunk->state = BOUND;
}

void chunk_render(struct Chunk *chunk, shader_t shader, bool draw[6], bool pass) {
    shader_use(shader);
    shader_setInt(shader, "textureArray", 0);
    shader_setInt(shader, "normalArray", 1);
    shader_setInt(shader, "shadowMap", 2);
    shader_setInt(shader, "lightMap", 3);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, block_getDiffuseArrayTexture());

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D_ARRAY, block_getNormalArrayTexture());

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, light.depthMap.map);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_3D, chunk->lightMap.texture);

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
            if (chunk->mesh.opaque.meshes[i].indices.length > 0) {
                glBindVertexArray(chunk->VAO[i]);
                glDrawElements(GL_TRIANGLES, chunk->mesh.opaque.meshes[i].indices.length, GL_UNSIGNED_INT, 0);
            }
        }
    }
}
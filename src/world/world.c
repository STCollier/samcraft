#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "uthash/uthash.h"

#include "../engine/core/shader.h"
#include "../engine/core/camera.h"
#include "../engine/core/globals.h"
#include "../engine/util/util.h"
#include "../engine/util/types.h"
#include "../engine/util/thpool.h"
#include "../engine/func/player.h"

#include "worldgen.h"
#include "chunk.h"
#include "block.h"
#include "world.h"

struct World world;

static bool chunkPlaneIsVisible(size_t index, struct Chunk* chunk) {
    if (index == 0) {
        return player.chunkPosition[1] >= chunk->position[1];
    } else if (index == 1) {
        return player.chunkPosition[1] <= chunk->position[1];
    } else if (index == 2) {
        return player.chunkPosition[0] >= chunk->position[0];
    } else if (index == 3) {
        return player.chunkPosition[0] <= chunk->position[0];
    } else if (index == 4) {
        return player.chunkPosition[2] >= chunk->position[2];
    } else if (index == 5) {
        return player.chunkPosition[2] <= chunk->position[2];
    }
    
    return 1;
}

void world_addChunk(ivec3 position) {
    struct Chunk *chunk = malloc(sizeof(struct Chunk));

    if (chunk == NULL) {
        ERROR("Failed to allocate memory for chunk\n");
        exit(EXIT_FAILURE);
    }

    memset(chunk, 0, sizeof(struct Chunk));
    glm_ivec3_copy(position, chunk->position);
    chunk->key.x = position[0];
    chunk->key.y = position[1];
    chunk->key.z = position[2];

    HASH_ADD(hh, world.chunks, key, sizeof(chunk_key_t), chunk);

    chunk_init(chunk, position);
}

static void world_generateChunk(ivec3 position) {
    chunk_generate(world_getChunk(position));
}

static void world_deleteChunk(ivec3 position) {
    struct Chunk *chunk = world_getChunk(position);

    if (chunk != NULL) {
        HASH_DEL(world.chunks, chunk);
        free(chunk);
    }
}

struct Chunk *world_getChunk(ivec3 position) {
    struct Chunk *chunk = NULL;

    // Create a temporary key to use for searching
    chunk_key_t temp_key;
    temp_key.x = position[0];
    temp_key.y = position[1];
    temp_key.z = position[2];

    // Search for the chunk with the given position
    HASH_FIND(hh, world.chunks, &temp_key, sizeof(chunk_key_t), chunk);

    if (chunk == NULL) {
        //fprintf(stderr, "Failed to find chunk at (%d %d %d)\n", position[0], position[1], position[2]);
        return NULL;
    }

    return chunk;
}

uint8_t getBlockFromWorldPosition(int x, int y, int z) {
    ivec3 worldPosition, chunkPosition, blockPosition;
    glm_ivec3_copy((ivec3){x, y, z}, worldPosition);
    glm_ivec3_copy((ivec3) {
        worldPosition[0] / CHUNK_SIZE,
        worldPosition[1] / CHUNK_SIZE,
        worldPosition[2] / CHUNK_SIZE},
    chunkPosition);

    for (int i = 0; i < 3; i++) {
        if (blockPosition[i] < 1) {
            blockPosition[i] += CHUNK_SIZE;
        }
    }

    glm_ivec3_copy((ivec3) {
        (worldPosition[0] % CHUNK_SIZE) + 1,
        (worldPosition[1] % CHUNK_SIZE) + 1,
        (worldPosition[2] % CHUNK_SIZE) + 1},
    blockPosition);

    for (int i = 0; i < 3; i++) {
        if (blockPosition[i] < 1) {
            blockPosition[i] += CHUNK_SIZE;
            chunkPosition[i] -= 1;
        }
    }

    struct Chunk* chunk = world_getChunk(chunkPosition);
    if (chunk == NULL)
        return BLOCK_AIR; // this is fine (just means chunks and thus colliders haven't loaded yet)

    return chunk->voxels[blockIndex(blockPosition[0], blockPosition[1], blockPosition[2])];
}

void world_meshChunk(ivec3 position) {
    struct Chunk *chunk = world_getChunk(position);

    chunk_mesh(chunk);
    chunk_bind(chunk);
}

void world_remeshChunk(ivec3 position) {
    struct Chunk *chunk = world_getChunk(position);

    if (!chunk->empty) {
        /*uint64_t_arr_delete(&chunk->mesh->opaque->vertices);
        uint32_t_arr_delete(&chunk->mesh->opaque->indices);

        uint64_t_arr_delete(&chunk->mesh->transparent->vertices);
        uint32_t_arr_delete(&chunk->mesh->transparent->indices);*/

        ivec3 positions[6] = {
            {position[0] + 1, position[1], position[2]}, // Right
            {position[0] - 1, position[1], position[2]}, // Left
            {position[0], position[1] + 1, position[2]}, // Top
            {position[0], position[1] - 1, position[2]}, // Bottom
            {position[0], position[1], position[2] + 1}, // Front
            {position[0], position[1], position[2] - 1}, // Back
        };

        chunk_remesh(world_getChunk(position), 
            world_getChunk(positions[RIGHT]),
            world_getChunk(positions[LEFT]),
            world_getChunk(positions[TOP]),
            world_getChunk(positions[BOTTOM]),
            world_getChunk(positions[FRONT]),
            world_getChunk(positions[BACK])
        );

        chunk_bind(chunk);
    } else {
        chunk_mesh(chunk);

        ivec3 positions[6] = {
            {position[0] + 1, position[1], position[2]}, // Right
            {position[0] - 1, position[1], position[2]}, // Left
            {position[0], position[1] + 1, position[2]}, // Top
            {position[0], position[1] - 1, position[2]}, // Bottom
            {position[0], position[1], position[2] + 1}, // Front
            {position[0], position[1], position[2] - 1}, // Back
        };

        chunk_remesh(world_getChunk(position), 
            world_getChunk(positions[RIGHT]),
            world_getChunk(positions[LEFT]),
            world_getChunk(positions[TOP]),
            world_getChunk(positions[BOTTOM]),
            world_getChunk(positions[FRONT]),
            world_getChunk(positions[BACK])
        );

        chunk_bind(chunk);
    }
}

static void world_addChunkToQueue(struct Chunk *chunk, enum ChunkQueueState state) {
    if (state == GENERATE) {
        if (world.chunkQueue.toGenerate.size < world.chunkQueue.toGenerate.capacity) {
            world.chunkQueue.toGenerate.chunks[world.chunkQueue.toGenerate.size] = chunk;
        } else {
            world.chunkQueue.toGenerate.capacity *= 2;
            world.chunkQueue.toGenerate.chunks = realloc(world.chunkQueue.toGenerate.chunks, world.chunkQueue.toGenerate.capacity * sizeof(struct Chunk));
            world.chunkQueue.toGenerate.chunks[world.chunkQueue.toGenerate.size] = chunk;
        }

        world.chunkQueue.toGenerate.size++;
    } else if (state == MESH) {
        if (world.chunkQueue.toMesh.size < world.chunkQueue.toMesh.capacity) {
            world.chunkQueue.toMesh.chunks[world.chunkQueue.toMesh.size] = chunk;
        } else {
            world.chunkQueue.toMesh.capacity *= 2;
            world.chunkQueue.toMesh.chunks = realloc(world.chunkQueue.toMesh.chunks, world.chunkQueue.toMesh.capacity * sizeof(struct Chunk));
            world.chunkQueue.toMesh.chunks[world.chunkQueue.toMesh.size] = chunk;
        }

        world.chunkQueue.toMesh.size++;
    } else ERROR("Invalid ChunkQueueState at world_addChunkToQueue");
}

static void world_clearChunkQueue(enum ChunkQueueState state) {
    if (state == GENERATE) {
        world.chunkQueue.toGenerate.size = 0;
        world.chunkQueue.toGenerate.capacity = 16;

        memset(world.chunkQueue.toGenerate.chunks, 0, world.chunkQueue.toGenerate.capacity);

        world.chunkQueue.toGenerate.chunks = realloc(world.chunkQueue.toGenerate.chunks, world.chunkQueue.toGenerate.capacity * sizeof(struct Chunk));

        world.chunkQueue.toGenerate.index = 0;
        world.chunkQueue.toGenerate.tick = 0;
    } else if (state == MESH) {
        world.chunkQueue.toMesh.size = 0;
        world.chunkQueue.toMesh.capacity = 16;

        memset(world.chunkQueue.toMesh.chunks, 0, world.chunkQueue.toMesh.capacity);

        world.chunkQueue.toMesh.chunks = realloc(world.chunkQueue.toMesh.chunks, world.chunkQueue.toMesh.capacity * sizeof(struct Chunk));

        world.chunkQueue.toMesh.index = 0;
        world.chunkQueue.toMesh.tick = 0;
    } else ERROR("Invalid ChunkQueueState at world_clearChunkQueue");
}

void thread_genchunk(void *arg) {
    struct Chunk *chunk = arg;
    //printf("Thread %p generating\n", pthread_self());
    chunk_generate(chunk);
}

void world_loadNewChunks() {
    for (int z = -world.renderRadius + player.chunkPosition[2]; z < world.renderRadius + player.chunkPosition[2]; z++) {
        for (int y = -world.renderHeight + player.chunkPosition[1]; y < world.renderHeight + player.chunkPosition[1]; y++) {
            for (int x = -world.renderRadius + player.chunkPosition[0]; x < world.renderRadius + player.chunkPosition[0]; x++) {
                ivec2 origin = {player.chunkPosition[0], player.chunkPosition[2]};
                ivec2 pos = {x, z};
                
                if (idist2d(origin, pos) < world.renderRadius) {
                    if (world_getChunk((ivec3){x, y, z}) == NULL) {
                        world_addChunk((ivec3){x, y, z});
                        world_addChunkToQueue(world_getChunk((ivec3){x, y, z}), GENERATE);
                    }
                }
            }
        }
    }

    for (int z = -world.renderRadius + player.chunkPosition[2]; z < world.renderRadius + player.chunkPosition[2]; z++) {
        for (int y = -world.renderHeight + player.chunkPosition[1]; y < world.renderHeight + player.chunkPosition[1]; y++) {
            for (int x = -world.renderRadius + player.chunkPosition[0]; x < world.renderRadius + player.chunkPosition[0]; x++) {
                ivec2 origin = {player.chunkPosition[0], player.chunkPosition[2]};
                ivec2 pos = {x, z};
                
                if (idist2d(origin, pos) < world.renderRadius) {
                    if (world_getChunk((ivec3){x, y, z})->state == GENERATED && !world_getChunk((ivec3){x, y, z})->addedToMeshQueue && !world_getChunk((ivec3){x, y, z})->empty) {
                        world_getChunk((ivec3){x, y, z})->addedToMeshQueue = true;
                        world_addChunkToQueue(world_getChunk((ivec3){x, y, z}), MESH);
                    }
                }
            }
        }
    }
}

void world_init(int renderRadius) {
    world.chunks = NULL; // Initilize to NULL for hashtable
    world.renderRadius = renderRadius;
    world.renderHeight = 3;
    world.loaded = false;

    world.chunkQueue.passesPerFrame = 1;
    world.chunkQueue.queuesComplete = false;

    world.chunkQueue.toGenerate.capacity = 16;
    world.chunkQueue.toGenerate.size = 0;
    world.chunkQueue.toGenerate.index = 0;
    world.chunkQueue.toGenerate.tick = 0;
    world.chunkQueue.toGenerate.chunks = malloc(sizeof(struct Chunk) * world.chunkQueue.toGenerate.capacity);
    memset(world.chunkQueue.toGenerate.chunks, 0, world.chunkQueue.toGenerate.capacity);

    world.chunkQueue.toMesh.capacity = 16;
    world.chunkQueue.toMesh.size = 0;
    world.chunkQueue.toMesh.index = 0;
    world.chunkQueue.toMesh.tick = 0;
    world.chunkQueue.toMesh.chunks = malloc(sizeof(struct Chunk) * world.chunkQueue.toMesh.capacity);
    memset(world.chunkQueue.toMesh.chunks, 0, world.chunkQueue.toMesh.capacity);
    
    worldgenInit(123);

    int c = 0;
    for (int z = -3; z < 3; z++) {
        for (int y = -3; y < 3; y++) {
            for (int x = -3; x < 3; x++) {
                ivec2 origin = {0, 0};
                ivec2 pos = {x, z};

                if (idist2d(origin, pos) < world.renderRadius) {
                    world_addChunk((ivec3){x, y, z});
                    world_generateChunk((ivec3){x, y, z});
                    c++;
                }
            }
        }
    }

    for (int z = -3; z < 3; z++) {
        for (int y = -3; y < 3; y++) {
            for (int x = -3; x < 3; x++) {
                ivec2 origin = {0, 0};
                ivec2 pos = {x, z};
                if (idist2d(origin, pos) < world.renderRadius && !world_getChunk((ivec3){x, y, z})->empty) {
                    world_meshChunk((ivec3){x, y, z});
                }
            }
        }
    }

    world.loaded = true;
    LOG_IMSG("World loaded, Chunk count: ", c);
}

void world_update(threadpool thpool) {
    if (player.exitedChunk) {
        world_loadNewChunks();

        player.exitedChunk = false;
    }

    //printf("gen size: %zu\nmesh size: %zu\n\n", world.chunkQueue.toGenerate.size, world.chunkQueue.toMesh.size);

    if (world.chunkQueue.toGenerate.size > 0) {
        if (world.chunkQueue.toGenerate.index < world.chunkQueue.toGenerate.size) {
            if (world.chunkQueue.toGenerate.tick % 2) {
                for (int i = 0; i < world.chunkQueue.passesPerFrame; i++) {
                    if (world.chunkQueue.toGenerate.index + i < world.chunkQueue.toGenerate.size) {
                        thpool_add_work(thpool, thread_genchunk, world.chunkQueue.toGenerate.chunks[world.chunkQueue.toGenerate.index + i]);
                    }
                }
                world.chunkQueue.toGenerate.index += world.chunkQueue.passesPerFrame;
            }
        } else world_clearChunkQueue(GENERATE);

        world.chunkQueue.toGenerate.tick++;
    }

    thpool_wait(thpool);

    if (world.chunkQueue.toMesh.size > 0) {
        if (world.chunkQueue.toMesh.index < world.chunkQueue.toMesh.size) {
            if (world.chunkQueue.toMesh.tick) {
                for (int i = 0; i < world.chunkQueue.passesPerFrame; i++) {
                    if (world.chunkQueue.toMesh.index + i < world.chunkQueue.toMesh.size) {
                        world_meshChunk(world.chunkQueue.toMesh.chunks[world.chunkQueue.toMesh.index + i]->position);
                    }
                }
                world.chunkQueue.toMesh.index += world.chunkQueue.passesPerFrame;
            }
        } else world_clearChunkQueue(MESH);

        world.chunkQueue.toMesh.tick++;
    }

    world.chunkQueue.queuesComplete = (world.chunkQueue.toGenerate.size == 0 && world.chunkQueue.toMesh.size == 0);
}

void world_render(shader_t shader, struct Frustum frustum, int pass) {
    if (pass == 1) {
        int passed = 0, total = 0, culled = 0;
        for (int z = -world.renderRadius + player.chunkPosition[2]; z < world.renderRadius + player.chunkPosition[2]; z++) {
            for (int y = -world.renderHeight + player.chunkPosition[1]; y < world.renderHeight + player.chunkPosition[1]; y++) {
                for (int x = -world.renderRadius + player.chunkPosition[0]; x < world.renderRadius + player.chunkPosition[0]; x++) {
                    ivec2 origin = {player.chunkPosition[0], player.chunkPosition[2]};
                    ivec2 pos = {x, z};

                    struct Chunk* chunk = world_getChunk((ivec3){x, y, z});

                    if (idist2d(origin, pos) < world.renderRadius && chunk != NULL && chunk->state == BOUND) {
                        if (boxInFrustum(frustum, *chunk)) {
                            bool draw[6];
                            for (int i = 0; i < 6; i++) {
                                bool willRender = chunkPlaneIsVisible(i, chunk);
                                draw[i] = willRender;
                                if (willRender) culled++;
                            }

                            chunk_render(chunk, shader, draw, 1);
                            passed++;
                        }
                        total++;
                    }
                }
            }
        }
        
        //printf("Rendered: %d/%d   Culled: %d/%d chunk faces\n", passed, total, culled, passed*6);
    } else {
        glCullFace(GL_FRONT);
        for (int z = -world.renderRadius + player.chunkPosition[2]; z < world.renderRadius + player.chunkPosition[2]; z++) {
            for (int y = -world.renderHeight + player.chunkPosition[1]; y < world.renderHeight + player.chunkPosition[1]; y++) {
                for (int x = -world.renderRadius + player.chunkPosition[0]; x < world.renderRadius + player.chunkPosition[0]; x++) {
                    ivec2 origin = {player.chunkPosition[0], player.chunkPosition[2]};
                    ivec2 pos = {x, z};

                    struct Chunk* chunk = world_getChunk((ivec3){x, y, z});

                    if (idist2d(origin, pos) < world.renderRadius && chunk != NULL && chunk->state == BOUND) {
                        bool draw[6] = {1, 1, 1, 1, 1, 1};
                        chunk_render(chunk, shader, draw, 1);
                    }
                }
            }
        }
    }
    glCullFace(GL_BACK);
}
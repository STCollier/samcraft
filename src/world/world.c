#include "world.h"
#include "worldgen.h"
#include "../engine/player.h"
#include "../engine/globals.h"

struct World world;

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

void world_meshChunk(ivec3 position) {
    struct Chunk *chunk = world_getChunk(position);

    chunk_mesh(chunk);
    chunk_bind(chunk);
}

void world_remeshChunk(ivec3 position) {
    struct Chunk *chunk = world_getChunk(position);

    if (!chunk->empty) {
        uint64_t_arr_delete(&chunk->mesh->opaque->vertices);
        uint32_t_arr_delete(&chunk->mesh->opaque->indices);

        uint64_t_arr_delete(&chunk->mesh->transparent->vertices);
        uint32_t_arr_delete(&chunk->mesh->transparent->indices);

        ivec3 positions[6] = {
            {position[0] + 1, position[1], position[2]}, // Right
            {position[0] - 1, position[1], position[2]}, // Left
            {position[0], position[1], position[2] - 1}, // Front
            {position[0], position[1], position[2] + 1}, // Back
            {position[0], position[1] + 1, position[2]}, // Top
            {position[0], position[1] - 1, position[2]}, // Bottom
        };

        chunk_remesh(world_getChunk(position), 
            world_getChunk(positions[RIGHT]),
            world_getChunk(positions[LEFT]),
            world_getChunk(positions[FRONT]),
            world_getChunk(positions[BACK]),
            world_getChunk(positions[TOP]),
            world_getChunk(positions[BOTTOM])
        );

        chunk_bind(chunk);
    } else {
        chunk_mesh(chunk);

        ivec3 positions[6] = {
            {position[0] + 1, position[1], position[2]}, // Right
            {position[0] - 1, position[1], position[2]}, // Left
            {position[0], position[1], position[2] - 1}, // Front
            {position[0], position[1], position[2] + 1}, // Back
            {position[0], position[1] + 1, position[2]}, // Top
            {position[0], position[1] - 1, position[2]}, // Bottom
        };

        chunk_remesh(world_getChunk(position), 
            world_getChunk(positions[RIGHT]),
            world_getChunk(positions[LEFT]),
            world_getChunk(positions[FRONT]),
            world_getChunk(positions[BACK]),
            world_getChunk(positions[TOP]),
            world_getChunk(positions[BOTTOM])
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
    
    worldgenInit(0x1);

    for (int z = -world.renderRadius; z < world.renderRadius; z++) {
        for (int y = -world.renderHeight; y < world.renderHeight; y++) {
            for (int x = -world.renderRadius; x < world.renderRadius; x++) {
                ivec2 origin = {0, 0};
                ivec2 pos = {x, z};

                if (idist2d(origin, pos) < world.renderRadius) {
                    world_addChunk((ivec3){x, y, z});
                    world_generateChunk((ivec3){x, y, z});
                }
            }
        }
    }

    for (int z = -world.renderRadius; z < world.renderRadius; z++) {
        for (int y = -world.renderHeight; y < world.renderHeight; y++) {
            for (int x = -world.renderRadius; x < world.renderRadius; x++) {
                ivec2 origin = {0, 0};
                ivec2 pos = {x, z};

                if (idist2d(origin, pos) < world.renderRadius && !world_getChunk((ivec3){x, y, z})->empty) {
                    world_meshChunk((ivec3){x, y, z});
                }
            }
        }
    }

    LOG("World loaded!");
}

ivec3s renderPos = (ivec3s){0, 0, 0};

float t = 0;
int fps = 0;
void world_render(shader_t shader, threadpool thpool) {
    if (player.exitedChunk) {
        world_loadNewChunks();

        player.exitedChunk = false;
    }

    t += window.dt;
    if (t >= 1) {
        world_loadNewChunks();
        printf("FPS: %d\n", fps);
        t = 0;
        fps = 0;
    } else {
        fps++;
    }

    //printf("gen size: %zu\nmesh size: %zu\n\n", world.chunkQueue.toGenerate.size, world.chunkQueue.toMesh.size);

    if (world.chunkQueue.toGenerate.size > 0) {
        if (world.chunkQueue.toGenerate.index < world.chunkQueue.toGenerate.size) {
            if (world.chunkQueue.toGenerate.tick) {
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

    for (int z = -world.renderRadius + player.chunkPosition[2]; z < world.renderRadius + player.chunkPosition[2]; z++) {
        for (int y = -world.renderHeight + player.chunkPosition[1]; y < world.renderHeight + player.chunkPosition[1]; y++) {
            for (int x = -world.renderRadius + player.chunkPosition[0]; x < world.renderRadius + player.chunkPosition[0]; x++) {
                ivec2 origin = {player.chunkPosition[0], player.chunkPosition[2]};
                ivec2 pos = {x, z};

                if (idist2d(origin, pos) < world.renderRadius && world_getChunk((ivec3){x, y, z}) != NULL) {
                    if (world_getChunk((ivec3){x, y, z})->state == BOUND) chunk_render(world_getChunk((ivec3){x, y, z}), shader, 1);
                }
            }
        }
    }

    for (int z = -world.renderRadius + player.chunkPosition[2]; z < world.renderRadius + player.chunkPosition[2]; z++) {
        for (int y = -world.renderHeight + player.chunkPosition[1]; y < world.renderHeight + player.chunkPosition[1]; y++) {
            for (int x = -world.renderRadius + player.chunkPosition[0]; x < world.renderRadius + player.chunkPosition[0]; x++) {
                ivec2 origin = {player.chunkPosition[0], player.chunkPosition[2]};
                ivec2 pos = {x, z};

                if (idist2d(origin, pos) < world.renderRadius && world_getChunk((ivec3){x, y, z}) != NULL) {
                    if (world_getChunk((ivec3){x, y, z})->state == BOUND) chunk_render(world_getChunk((ivec3){x, y, z}), shader, 0);
                }
            }
        }
    }
}
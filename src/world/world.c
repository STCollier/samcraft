#include "world.h"
#include "worldgen.h"

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
    chunk_generate(chunk);
}

static void world_deleteChunk(struct Chunk *chunk) {
    HASH_DEL(world.chunks, chunk);
    free(chunk);
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
        fprintf(stderr, "Failed to find chunk at (%d %d %d)\n", position[0], position[1], position[2]);
        return NULL;
    }

    return chunk;
}

void world_meshChunk(ivec3 position) {
    ivec3 positions[6] = {
        {position[0] + 1, position[1], position[2]}, // Right
        {position[0] - 1, position[1], position[2]}, // Left
        {position[0], position[1], position[2] - 1}, // Front
        {position[0], position[1], position[2] + 1}, // Back
        {position[0], position[1] + 1, position[2]}, // Top
        {position[0], position[1] - 1, position[2]}, // Bottom
    };

    chunk_mesh(world_getChunk(position), 
        world_getChunk(positions[RIGHT]),
        world_getChunk(positions[LEFT]),
        world_getChunk(positions[FRONT]),
        world_getChunk(positions[BACK]),
        world_getChunk(positions[TOP]),
        world_getChunk(positions[BOTTOM])
    );

    chunk_bind(world_getChunk(position));
}

void world_remeshChunk(ivec3 position) {
    struct Chunk *chunk = world_getChunk(position);

    free(chunk->vertexList->opaque->data);
    chunk->vertexList->opaque->size = 0;

    free(chunk->vertexList->transparent->data);
    chunk->vertexList->transparent->size = 0;

    world_meshChunk(position);
}


void world_init(int renderRadius) {
    world.chunks = NULL; // Initilize to NULL for hashtable
    world.renderRadius = renderRadius;
    world.chunkRenderDepth = 3;
    
    worldgenInit(0x1);

    int generationRadius = renderRadius + 1;

    int added = 0;
    for (int z = -generationRadius; z < generationRadius; z++) {
        for (int y = -1; y <= world.chunkRenderDepth; y++) {
            for (int x = -generationRadius; x < generationRadius; x++) {
                ivec2 origin = {0, 0};
                ivec2 pos = {x, z};

                //if (idist2d(origin, pos) <= world.renderRadius)
                EXECTIME("world_addChunk", added,
                    world_addChunk((ivec3){x, y, z});
                )

                added++;
            }
        }
    }

    int chunkCount = 0;
    for (int z = -world.renderRadius; z < world.renderRadius; z++) {
        for (int y = 0; y < world.chunkRenderDepth; y++) {
            for (int x = -world.renderRadius; x < world.renderRadius; x++) {
                ivec2 origin = {0, 0};
                ivec2 pos = {x, z};

                //if (idist2d(origin, pos) < world.renderRadius) 

                EXECTIME("world_meshChunk", chunkCount,
                    world_meshChunk((ivec3){x, y, z});
                )
                chunkCount++;
            }
        }
    }

    LOG_IMSG("World loaded! Chunk count:", chunkCount);
}

void world_render(shader_t shader) {
    for (int z = -world.renderRadius; z < world.renderRadius; z++) {
        for (int y = 0; y < world.chunkRenderDepth; y++) {
            for (int x = -world.renderRadius; x < world.renderRadius; x++) {
                ivec2 origin = {0, 0};
                ivec2 pos = {x, z};

                //if (idist2d(origin, pos) < world.renderRadius)
                chunk_render(world_getChunk((ivec3){x, y, z}), shader, 1);
            }
        }
    }

    for (int z = -world.renderRadius; z < world.renderRadius; z++) {
        for (int y = 0; y < world.chunkRenderDepth; y++) {
            for (int x = -world.renderRadius; x < world.renderRadius; x++) {
                ivec2 origin = {0, 0};
                ivec2 pos = {x, z};

                //if (idist2d(origin, pos) < world.renderRadius) 
                chunk_render(world_getChunk((ivec3){x, y, z}), shader, 0);
            }
        }
    }
}
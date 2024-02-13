#include "world.h"

#define WORLD_APOTHEM 1000
#define WORLD_LENGTH (WORLD_APOTHEM * 2)

struct World world;

int world_hashChunk(ivec2s position) {
    return (position.x + WORLD_APOTHEM) + (position.y + WORLD_APOTHEM) * WORLD_LENGTH;
}

ivec2s world_unhashChunk(int hash) {
    return (ivec2s){
        (hash % WORLD_LENGTH) - WORLD_APOTHEM,
        (hash / WORLD_LENGTH) - WORLD_APOTHEM
    };
}

static void world_addChunk(ivec2s position) {
    struct Chunk *chunk = malloc(sizeof(struct Chunk));
    int c_hash = world_hashChunk(position);

    HASH_FIND_INT(world.chunks, &c_hash, chunk);

    if (chunk == NULL) {
        chunk = malloc(sizeof(struct Chunk));

        chunk->id = c_hash;

        HASH_ADD_INT(world.chunks, id, chunk);
    }

    chunk_init(chunk, (ivec2){position.x, position.y});
    chunk_generate(chunk);
}

static void world_deleteChunk(struct Chunk *chunk) {
    HASH_DEL(world.chunks, chunk);
    free(chunk);
}

struct Chunk *world_getChunk(int id) {
    struct Chunk *chunk;

    HASH_FIND_INT(world.chunks, &id, chunk);  // chunk is output pointer

    if (chunk == NULL) {
        fprintf(stderr, "\x1B[0m%s:%d: \x1B[0;31m[ERROR]\x1B[0m %s %d (x: %d, y: %d)\n", __FILE__, __LINE__, "Could not locate chunk with ID", id, world_unhashChunk(id).x, world_unhashChunk(id).y);
        //exit(EXIT_FAILURE);
    }

    return chunk;
}

void world_meshChunk(ivec2s position) {
    struct Chunk chunkNeighbors[4];

    ivec2s positions[4] = {
        (ivec2s){position.x + 1, position.y}, // Right
        (ivec2s){position.x - 1, position.y}, // Left
        (ivec2s){position.x, position.y + 1}, // Front
        (ivec2s){position.x, position.y - 1}  // Back
    };

    for (int faces = 0; faces < 4; faces++) {
        chunkNeighbors[faces] = *world_getChunk(world_hashChunk(positions[faces]));
    }

    chunk_mesh(world_getChunk(world_hashChunk(position)), chunkNeighbors);
    chunk_bind(world_getChunk(world_hashChunk(position)));
}


void world_init(int renderRadius) {
    world.chunks = NULL; // Initilize to NULL for hashtable
    world.renderRadius = renderRadius;

    for (int y = -renderRadius; y <= renderRadius; y++) {
        for (int x = -renderRadius; x <= renderRadius; x++) {
            ivec2s origin = (ivec2s){0, 0};
            ivec2s currentPoint = (ivec2s){x, y};

            if (idist(currentPoint, origin) <= renderRadius) {
                world_addChunk(currentPoint);
            }
        }
    }

    int chunkCount = 0;

    for (int y = -renderRadius; y < renderRadius; y++) {
        for (int x = -renderRadius; x < renderRadius; x++) {
            ivec2s origin = (ivec2s){0, 0};
            ivec2s currentPoint = (ivec2s){x, y};

            if (idist(currentPoint, origin) < renderRadius) {
                world_meshChunk(currentPoint);
                chunkCount++;
            }
        }
    }

    LOG_IMSG("World loaded! Chunk count:", chunkCount);
}

void world_render(shader_t shader) {
    for (int y = -world.renderRadius; y < world.renderRadius; y++) {
        for (int x = -world.renderRadius; x < world.renderRadius; x++) {
            ivec2s origin = (ivec2s){0, 0};
            ivec2s currentPoint = (ivec2s){x, y};

            if (idist(currentPoint, origin) < world.renderRadius) {
                chunk_render(world_getChunk(world_hashChunk(currentPoint)), shader);
            }
        }
    }
}
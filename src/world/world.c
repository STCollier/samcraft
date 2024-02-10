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
        exit(EXIT_FAILURE);
    }

    return chunk;
}

void world_meshChunk(ivec2s position) {
    struct Chunk chunkNeighbors[4];

    ivec2s positions[4] = {
        (ivec2s){position.x + 1, position.y}, // Right
        (ivec2s){position.x - 1, position.y}, // Left
        (ivec2s){position.x, position.y + 1}, // Front
        (ivec2s){position.x, position.y + 1}  // Back
    };

    for (int faces = 0; faces < 4; faces++) {
        chunkNeighbors[faces] = *world_getChunk(world_hashChunk(positions[faces]));
    }

    chunk_mesh(world_getChunk(world_hashChunk(position)), chunkNeighbors);
    chunk_bind(world_getChunk(world_hashChunk(position)));
}


void world_init() {
    world.chunks = NULL; // Initilize to NULL for hashtable

    for (int y = -1; y < 11; y++) {
        for (int x = -1; x < 11; x++) {
            world_addChunk((ivec2s){x, y});
        }
    }

    for (int y = 0; y < 10; y++) {
        for (int x = 0; x < 10; x++) {
            world_meshChunk((ivec2s){x, y});
        }
    }

    LOG("World loaded!");
}

void world_render(shader_t shader) {
    for (int y = 0; y < 10; y++) {
        for (int x = 0; x < 10; x++) {
            chunk_render(world_getChunk(world_hashChunk((ivec2s){x, y})), shader);
        }
    }
}
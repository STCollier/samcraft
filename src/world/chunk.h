#ifndef CHUNK_H
#define CHUNK_H

#include "cglm/cglm.h"
#include "cglm/call.h"

#include "../game/shader.h"
#include "../game/camera.h"
#include "../game/util.h"

#define CHUNK_SIZE_X 16
#define CHUNK_SIZE_Y 256
#define CHUNK_SIZE_Z 16

#define CHUNK_MEMORY_BUFFER 1000000

typedef enum {
    RIGHT,
    LEFT,
    FRONT,
    BACK,
    TOP,
    BOTTOM
} Direction;


struct Block {
    int id;
};

struct Chunk {
    unsigned int blockTexture;
    unsigned int VBO, VAO;

    struct Block block[CHUNK_SIZE_X][CHUNK_SIZE_Y][CHUNK_SIZE_Z];
    size_t meshSize;
    vec3 offset;
    float meshData[CHUNK_MEMORY_BUFFER];
};

void initChunk(struct Chunk *chunk, vec3 offset);
void constructChunkMesh(struct Chunk *chunk, struct Chunk *chunkNeighbors);
void loadChunk(struct Chunk *chunk);
void renderChunk(struct Chunk *chunk, struct Shader shader);
void destroyChunk(struct Chunk *chunk);

#endif
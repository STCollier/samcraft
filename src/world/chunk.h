#ifndef CHUNK_H
#define CHUNK_H

#include "cglm/cglm.h"
#include "cglm/call.h"

#include "../game/shader.h"
#include "../game/camera.h"
#include "../game/util.h"

#include <stdbool.h>

#include "block.h"

#define CHUNK_SIZE_X 16
#define CHUNK_SIZE_Y 256
#define CHUNK_SIZE_Z 16
#define CHUNK_AREA CHUNK_SIZE_X * CHUNK_SIZE_Y * CHUNK_SIZE_Z

#define CHUNK_MEMORY_BUFFER 1000000

struct Chunk {
    unsigned int arrayTexture;
    unsigned int VBO, VAO;

    struct Block *blocks;
    size_t meshSize;
    ivec2 offset;
    ivec2 worldPos;
    float *meshData;
    bool isNull;
};

struct ChunkData {
    int worldPosX;
    int worldPosY;
    unsigned char *data;
};

void initChunk(struct Chunk *chunk, ivec2 offset);
void genChunk(struct Chunk *chunk);
void meshChunk(struct Chunk *chunk, struct Chunk *chunkNeighbors);
void bindChunk(struct Chunk *chunk);
void renderChunk(struct Chunk *chunk, struct Shader shader);
void destroyChunk(struct Chunk *chunk);
int blockIndex(int x, int y, int z);

#endif
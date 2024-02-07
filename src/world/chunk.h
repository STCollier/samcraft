#ifndef CHUNK_H
#define CHUNK_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "cglm/cglm.h"
#include "cglm/call.h"
#include "uthash/uthash.h"

#include <stdbool.h>
#include <string.h>

#include "../engine/shader.h"
#include "../engine/camera.h"
#include "../engine/util.h"
#include "../engine/types.h"
#include "block.h"

#define CHUNK_SIZE_X 16
#define CHUNK_SIZE_Y 256
#define CHUNK_SIZE_Z 16
#define CHUNK_AREA CHUNK_SIZE_X * CHUNK_SIZE_Y * CHUNK_SIZE_Z

#define CHUNK_MEMORY_BUFFER 36000

struct Chunk {
    unsigned int arrayTexture;
    unsigned int VBO, VAO;

    struct Block *blocks;
    size_t meshSize;
    ivec2 offset; // Offset in blocks (16, 32, 48, etc.)
    ivec2 worldPos; // Offset in chunks (1, 2, 3, etc.)
    uint32_t *meshData;
    bool isNull;

    int id; // Key, represented as a 1D index in a 2D array
    UT_hash_handle hh; // Makes this structure hashable
};

struct ChunkData {
    int worldPosX;
    int worldPosY;
    unsigned char *data;
};

void chunk_init(struct Chunk *chunk, ivec2 position);
void chunk_generate(struct Chunk *chunk);
void chunk_mesh(struct Chunk *chunk, struct Chunk *chunkNeighbors);
void chunk_bind(struct Chunk *chunk);
void chunk_render(struct Chunk *chunk, shader_t shader);
void chunk_destroy(struct Chunk *chunk);
int blockIndex(int x, int y, int z);

#endif
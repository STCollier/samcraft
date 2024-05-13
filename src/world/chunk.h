#ifndef CHUNK_H
#define CHUNK_H

#include "../engine/core/shader.h"
#include "../engine/util/common.h"
#include "uthash/uthash.h"

#define CHUNK_SIZE 50
#define WATER_HEIGHT 0
#define CHUNK_AREA CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE

typedef struct {
    int x;
    int y;
    int z;
} chunk_key_t;

enum ChunkPipelineState {
    ADDED,
    GENERATED,
    MESHED,
    BOUND,
};

struct ChunkMesh {
    struct MeshData *opaque;
    struct MeshData *transparent;
};

struct Chunk {
    uint8_t *voxels;
    ivec3 position; // chunk location relative to to other chunks e.g (0, 0, 0) origin
    chunk_key_t key; // Hashtable key
    UT_hash_handle hh; // Makes this structure hashable

    struct ChunkMesh *mesh;
    enum ChunkPipelineState state;
    bool addedToMeshQueue;
    bool empty;

    unsigned int arrayTexture;
    unsigned int VBO, VAO, EBO, tVBO, tVAO, tEBO;
};

int blockIndex(int x, int y, int z);

void chunk_init(struct Chunk *chunk, ivec3 pos);
void chunk_generate(struct Chunk *chunk);
void chunk_mesh(struct Chunk *chunk);
void chunk_remesh(struct Chunk *chunk, struct Chunk* cn_right, struct Chunk* cn_left, struct Chunk* cn_front, struct Chunk* cn_back, struct Chunk* cn_top, struct Chunk* cn_bottom);
void chunk_bind(struct Chunk *chunk);
void chunk_render(struct Chunk *chunk, shader_t shader, bool pass);

#endif
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
#include "../engine/mesher.h"
#include "block.h"

#define CHUNK_SIZE 50
#define CHUNK_AREA CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE

typedef struct {
    int x;
    int y;
    int z;
} chunk_key_t;

struct Chunk {
    bool isNull;
    unsigned int arrayTexture;
    unsigned int VBO, VAO;

    uint8_t *voxels;
    uint8_t *light_map;
    vertices_t *vertexList;

    ivec3 position; // chunk location relative to to other chunks e.g (0, 0, 0) origin

    chunk_key_t key; // Key, represented as a 1D index in a 2D array
    UT_hash_handle hh; // Makes this structure hashable s
};

int blockIndex(int x, int y, int z);

void chunk_init(struct Chunk *chunk, ivec3 pos);
void chunk_generate(struct Chunk *chunk);
void chunk_mesh(struct Chunk *chunk, struct Chunk* cn_right, struct Chunk* cn_left, struct Chunk* cn_front, struct Chunk* cn_back, struct Chunk* cn_top, struct Chunk* cn_bottom);
void chunk_bind(struct Chunk *chunk);
void chunk_render(struct Chunk *chunk, shader_t shader);

#endif
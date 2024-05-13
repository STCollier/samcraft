#ifndef MESHER_H
#define MESHER_H

#include "../util/common.h"

#define CS (CHUNK_SIZE)

// Padded chunk size
#define CS_P (CS + 2)
#define CS_2 (CS * CS)
#define CS_P2 (CS_P * CS_P)
#define CS_P3 (CS_P * CS_P * CS_P)

struct MeshData {
    arr_uint64_t vertices;
    arr_uint32_t indices;
};

struct MeshData *mesh(uint8_t *voxels, bool opaque);

#endif
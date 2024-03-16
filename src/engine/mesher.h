#ifndef MESHER_H
#define MESHER_H

#include "util.h"

#define CS (CHUNK_SIZE)

// Padded chunk size
#define CS_P (CS + 2)
#define CS_P2 (CS_P * CS_P)
#define CS_P3 (CS_P * CS_P * CS_P)

typedef struct {
    uint64_t *data;
    size_t size;
    size_t capacity;
} list64_t;

typedef struct {
    uint32_t *data;
    size_t size;
    size_t capacity;
} list32_t;

struct MeshData {
    list64_t *vertices;
    list32_t *indices;
};

int arr_at(uint8_t *arr, size_t idx);
struct MeshData *mesh(uint8_t *voxels, bool opaque);

#endif
#ifndef MESHER_H
#define MESHER_H

#include "util.h"

#define MAX_VERTEX_STORAGE 16000

#define CS (CHUNK_SIZE)

// Padded chunk size
#define CS_P (CS + 2)
#define CS_P2 (CS_P * CS_P)
#define CS_P3 (CS_P * CS_P * CS_P)

typedef struct {
    uint32_t x_y_z_type;
    uint16_t u_v;
    uint8_t norm_ao;
} vertex_t;

typedef struct {
    vertex_t *data;
    size_t size;
} vertices_t;

int arr_at(uint8_t *arr, size_t idx);
vertices_t *mesh(uint8_t *voxels, bool opaque);

#endif
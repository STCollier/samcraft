#ifndef MESHER_H
#define MESHER_H

#include "util.h"

#define MAX_VERTEX_STORAGE 36000

#define CS (CHUNK_SIZE)

// Padded chunk size
#define CS_P (CS + 2)
#define CS_P2 (CS_P * CS_P)
#define CS_P3 (CS_P * CS_P * CS_P)

typedef struct {
    uint32_t *data;
    size_t size;
} vertices_t;

int arr_at(uint8_t *arr, size_t idx);
vertices_t *mesh(uint8_t *voxels, uint8_t *light_map);
void calculate_light(uint8_t *voxels, uint8_t *light_map);

#endif
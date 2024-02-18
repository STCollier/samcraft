#include "mesher.h"
#include "../world/chunk.h"

#ifdef _MSC_VER
    static const int CTZ(uint64_t &x) {
        unsigned long index;
        _BitScanForward64(&index, x);
        return (int)index;
    }
#else
    static const int CTZ(uint64_t x) {
        return __builtin_ctzll(x);
    }
#endif

static const int get_axis_i(const int axis, const int a, const int b, const int c) {
    if (axis == 0) return b + (a * CS_P) + (c * CS_P2);
    else if (axis == 1) return a + (c * CS_P) + (b* CS_P2);
    else return c + (b * CS_P) + (a * CS_P2);
}

// Add checks to this function to skip culling against grass for example
static const bool solid_check(int voxel) {
    return voxel != BLOCK_AIR;
}

const ivec2 ao_dirs[8] = {
   {0, -1},
   {0, 1},
   {-1, 0},
   {1, 0},
   {-1, -1},
   {-1, 1},
   {1, -1},
   {1, 1},
};

static const int vertexAO(int side1, int side2, int corner) {
  if (side1 && side2) {
    return 0;
  }
  return 3 - (side1 + side2 + corner);
}

int arr_at(uint8_t *arr, size_t idx) {
    if (idx >= CS_P3) {
        ERROR("Index out of bounds at voxels_at()");
        exit(EXIT_FAILURE);
    }
    return arr[idx];
}

static const bool compare_ao(uint8_t *voxels, int axis, int forward, int right, int c, int forward_offset, int right_offset) {
  for (int i = 0; i < 8; i++) { // looping over ao_dirs
    if (solid_check(arr_at(voxels, get_axis_i(axis, right + ao_dirs[i][0], forward + ao_dirs[i][1], c))) !=
        solid_check(arr_at(voxels, get_axis_i(axis, right + right_offset + ao_dirs[i][0], forward + forward_offset + ao_dirs[i][1], c)))
    ) {
      return false;
    }
  }
  return true;
}

static const bool compare_forward(uint8_t *voxels, uint8_t *light_map, int axis, int forward, int right, int bit_pos, int light_dir) {
  return
    arr_at(voxels, get_axis_i(axis, right, forward, bit_pos)) == arr_at(voxels, get_axis_i(axis, right, forward + 1, bit_pos)) &&
    arr_at(light_map, get_axis_i(axis, right, forward, bit_pos + light_dir)) == arr_at(light_map, get_axis_i(axis, right, forward + 1, bit_pos + light_dir)) &&
    compare_ao(voxels, axis, forward, right, bit_pos + light_dir, 1, 0);
}

static const bool compare_right(uint8_t *voxels, uint8_t* light_map, int axis, int forward, int right, int bit_pos, int light_dir) {
  return
    arr_at(voxels, get_axis_i(axis, right, forward, bit_pos)) == arr_at(voxels, get_axis_i(axis, right + 1, forward, bit_pos)) &&
    arr_at(light_map, get_axis_i(axis, right, forward, bit_pos + light_dir)) == arr_at(light_map, get_axis_i(axis, right + 1, forward, bit_pos + light_dir)) &&
    compare_ao(voxels, axis, forward, right, bit_pos + light_dir, 0, 1);
}

static void insert_quad(vertices_t *vertexList, uint32_t v1, uint32_t v2, uint32_t v3, uint32_t v4, bool flipped) {
    if (flipped) {
        vertexList->data[vertexList->size++] = v1;
        vertexList->data[vertexList->size++] = v4;
        vertexList->data[vertexList->size++] = v2;
        vertexList->data[vertexList->size++] = v4;
        vertexList->data[vertexList->size++] = v2;
        vertexList->data[vertexList->size++] = v3;
    } else {
        vertexList->data[vertexList->size++] = v1;
        vertexList->data[vertexList->size++] = v2;
        vertexList->data[vertexList->size++] = v3;
        vertexList->data[vertexList->size++] = v3;
        vertexList->data[vertexList->size++] = v4;
        vertexList->data[vertexList->size++] = v1;
    }
}

static const uint32_t get_vertex(uint32_t x, uint32_t y, uint32_t z, uint32_t type, uint32_t light, uint32_t norm, uint32_t ao) {
    return (ao << 30) | (norm << 27) | (light << 23) | (type << 18) | ((z - 1) << 12) | ((y - 1) << 6) | (x - 1);
}

vertices_t *mesh(uint8_t *voxels, uint8_t *light_map) {
  uint64_t axis_cols[CS_P2 * 3] = { 0 };
  uint64_t col_face_masks[CS_P2 * 6];

  vertices_t *vertexList = malloc(sizeof(vertices_t));
  vertexList->data = malloc(sizeof(uint32_t) * MAX_VERTEX_STORAGE);
  vertexList->size = 0;


  // Step 1: Convert to binary representation for each direction
  int voxelIdx = 0;
  for (int y = 0; y < CS_P; y++) {
    for (int x = 0; x < CS_P; x++) {
      uint64_t zb = 0;
      for (int z = 0; z < CS_P; z++) {
        if (solid_check(voxels[voxelIdx])) {
          axis_cols[x + (z * CS_P)] |= 1ULL << y;
          axis_cols[z + (y * CS_P) + (CS_P2)] |= 1ULL << x;
          zb |= 1ULL << z;
        }
        voxelIdx++;
      }
      axis_cols[y + (x * CS_P) + (CS_P2 * 2)] = zb;
    }
  }

  // Step 2: Visible face culling
  for (int axis = 0; axis <= 2; axis++) {
    for (int i = 0; i < CS_P2; i++) {
      uint64_t col = axis_cols[(CS_P2 * axis) + i];
      col_face_masks[(CS_P2 * (axis * 2)) + i] = col & ~((col >> 1) | (1ULL << (CS_P - 1)));
      col_face_masks[(CS_P2 * (axis * 2 + 1)) + i] = col & ~((col << 1) | 1ULL);
    }
  }

  // Step 3: Greedy meshing
  for (int face = 0; face < 6; face++) {
    int axis = face / 2;
    int light_dir = face % 2 == 0 ? 1 : -1;

    int merged_forward[CS_P2] = { 0 };
    for (int forward = 1; forward < CS_P - 1; forward++) {
      uint64_t bits_walking_right = 0;
      int merged_right[CS_P] = { 0 };
      for (int right = 1; right < CS_P - 1; right++) {
        uint64_t bits_here = col_face_masks[right + (forward * CS_P) + (face * CS_P2)];
        uint64_t bits_forward = forward >= CS ? 0 : col_face_masks[right + (forward * CS_P) + (face * CS_P2) + CS_P];
        uint64_t bits_right = right >= CS ? 0 : col_face_masks[right + 1 + (forward * CS_P) + (face * CS_P2)];
        uint64_t bits_merging_forward = bits_here & bits_forward & ~bits_walking_right;
        uint64_t bits_merging_right = bits_here & bits_right;

        uint64_t copy_front = bits_merging_forward;
        while (copy_front) {
          int bit_pos = CTZ(copy_front);
          copy_front &= ~(1ULL << bit_pos);

          if (bit_pos == 0 || bit_pos == CS_P - 1) continue;

          if (compare_forward(voxels, light_map, axis, forward, right, bit_pos, light_dir)) {
            merged_forward[(right * CS_P) + bit_pos]++;
          }
          else {
            bits_merging_forward &= ~(1ULL << bit_pos);
          }
        }

        uint64_t bits_stopped_forward = bits_here & ~bits_merging_forward;
        while (bits_stopped_forward) {
          int bit_pos = CTZ(bits_stopped_forward);
          bits_stopped_forward &= ~(1ULL << bit_pos);

          // Discards faces from neighbor voxels
          if (bit_pos == 0 || bit_pos == CS_P - 1) continue;

          if (
            (bits_merging_right & (1ULL << bit_pos)) != 0 &&
            merged_forward[(right * CS_P) + bit_pos] == merged_forward[(right + 1) * CS_P + bit_pos] &&
            compare_right(voxels, light_map, axis, forward, right, bit_pos, light_dir)
          ) {
            bits_walking_right |= 1ULL << bit_pos;
            merged_right[bit_pos]++;
            merged_forward[(right * CS_P) + bit_pos] = 0;
            continue;
          }
          bits_walking_right &= ~(1ULL << bit_pos);

          uint8_t mesh_left = right - merged_right[bit_pos];
          uint8_t mesh_right = right + 1;
          uint8_t mesh_front = forward - merged_forward[(right * CS_P) + bit_pos];
          uint8_t mesh_back = forward + 1;
          uint8_t mesh_up = bit_pos + (face % 2 == 0 ? 1 : 0);

          uint8_t type = arr_at(voxels, get_axis_i(axis, right, forward, bit_pos));
          uint8_t light = arr_at(light_map, get_axis_i(axis, right, forward, bit_pos + light_dir));

          int c = bit_pos + light_dir;
          uint8_t ao_F = solid_check(arr_at(voxels, get_axis_i(axis, right, forward - 1, c))) ? 1 : 0;
          uint8_t ao_B = solid_check(arr_at(voxels, get_axis_i(axis, right, forward + 1, c))) ? 1 : 0;
          uint8_t ao_L = solid_check(arr_at(voxels, get_axis_i(axis, right - 1, forward, c))) ? 1 : 0;
          uint8_t ao_R = solid_check(arr_at(voxels, get_axis_i(axis, right + 1, forward, c))) ? 1 : 0;

          uint8_t ao_LFC = solid_check(arr_at(voxels, get_axis_i(axis, right - 1, forward - 1, c))) ? 1 : 0;
          uint8_t ao_LBC = solid_check(arr_at(voxels, get_axis_i(axis, right - 1, forward + 1, c))) ? 1 : 0;
          uint8_t ao_RFC = solid_check(arr_at(voxels, get_axis_i(axis, right + 1, forward - 1, c))) ? 1 : 0;
          uint8_t ao_RBC = solid_check(arr_at(voxels, get_axis_i(axis, right + 1, forward + 1, c))) ? 1 : 0;

          uint8_t ao_LB = vertexAO(ao_L, ao_B, ao_LBC);
          uint8_t ao_LF = vertexAO(ao_L, ao_F, ao_LFC);
          uint8_t ao_RB = vertexAO(ao_R, ao_B, ao_RBC);
          uint8_t ao_RF = vertexAO(ao_R, ao_F, ao_RFC);

          merged_forward[(right * CS_P) + bit_pos] = 0;
          merged_right[bit_pos] = 0;

          uint32_t v1, v2, v3, v4;
          if (face == 0) {
            v1 = get_vertex(mesh_left, mesh_up, mesh_front, type, light, face, ao_LF);
            v2 = get_vertex(mesh_left, mesh_up, mesh_back, type, light, face, ao_LB);
            v3 = get_vertex(mesh_right, mesh_up, mesh_back, type, light, face, ao_RB);
            v4 = get_vertex(mesh_right, mesh_up, mesh_front, type, light, face, ao_RF);
          }
          else if (face == 1) {
            v1 = get_vertex(mesh_left, mesh_up, mesh_back, type, light, face, ao_LB);
            v2 = get_vertex(mesh_left, mesh_up, mesh_front, type, light, face, ao_LF);
            v3 = get_vertex(mesh_right, mesh_up, mesh_front, type, light, face, ao_RF);
            v4 = get_vertex(mesh_right, mesh_up, mesh_back, type, light, face, ao_RB);
          }
          else if (face == 2) {
            v1 = get_vertex(mesh_up, mesh_front, mesh_left, type, light, face, ao_LF);
            v2 = get_vertex(mesh_up, mesh_back, mesh_left, type, light, face, ao_LB);
            v3 = get_vertex(mesh_up, mesh_back, mesh_right, type, light, face, ao_RB);
            v4 = get_vertex(mesh_up, mesh_front, mesh_right, type, light, face, ao_RF);
          }
          else if (face == 3) {
            v1 = get_vertex(mesh_up, mesh_back, mesh_left, type, light, face, ao_LB);
            v2 = get_vertex(mesh_up, mesh_front, mesh_left, type, light, face, ao_LF);
            v3 = get_vertex(mesh_up, mesh_front, mesh_right, type, light, face, ao_RF);
            v4 = get_vertex(mesh_up, mesh_back, mesh_right, type, light, face, ao_RB);
          }
          else if (face == 4) {
            v1 = get_vertex(mesh_front, mesh_left, mesh_up, type, light, face, ao_LF);
            v2 = get_vertex(mesh_back, mesh_left, mesh_up, type, light, face, ao_LB);
            v3 = get_vertex(mesh_back, mesh_right, mesh_up, type, light, face, ao_RB);
            v4 = get_vertex(mesh_front, mesh_right, mesh_up, type, light, face, ao_RF);
          }
          else if (face == 5) {
            v1 = get_vertex(mesh_back, mesh_left, mesh_up, type, light, face, ao_LB);
            v2 = get_vertex(mesh_front, mesh_left, mesh_up, type, light, face, ao_LF);
            v3 = get_vertex(mesh_front, mesh_right, mesh_up, type, light, face, ao_RF);
            v4 = get_vertex(mesh_back, mesh_right, mesh_up, type, light, face, ao_RB);
          }

          if (ao_LB + ao_RF > ao_RB + ao_LF) {
            insert_quad(vertexList, v1, v2, v3, v4, true);
          }
          else {
            insert_quad(vertexList, v1, v2, v3, v4, false);
          }
        }
      }
    }
  }

  if (vertexList->size == 0) {
      ERROR("Vertexlist is empty. Returning NULL");
      free(vertexList->data);
      return NULL;
  }

  return vertexList;
}

void calculate_light(uint8_t *voxels, uint8_t *light_map) {
    for (int y = CS_P; y--;) {
      for (int zx = 0; zx < CS_P2; zx++) {
        int i = (y * CS_P2) + zx;
        if (y == CS_P - 1) {
          light_map[i] = 15;
        }
        else {
          int light_above = arr_at(light_map, i + CS_P2);
          if (arr_at(voxels, i) == 0 && light_above > 0) {
            light_map[i] = light_above;
          }
          else {
            light_map[i] = 4;
          }
        }
      }
    }
  }
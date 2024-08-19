#include "../../world/chunk.h"
#include "../../world/block.h"
#include "mesher.h"

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
static inline const bool solid_check(int voxel) {
	return voxel != BLOCK_AIR;
}

const ivec2 ao_dirs[8] = {
   {-1, 0},
   {0, -1},
   {0, 1},
   {1, 0},
   {-1, -1},
   {-1, 1},
   {1, -1},
   {1, 1},
};

static inline const int vertexAO(int side1, int side2, int corner) {
  	return (side1 && side2) ? 0 : (3 - (side1 + side2 + corner));
}

static inline int arr_at(uint8_t *arr, size_t idx) {
    if (idx >= CS_P3) {
        ERROR("Index out of bounds at voxels_at()");
        exit(EXIT_FAILURE);
    }
    return arr[idx];
}

static inline const bool compare_ao(uint8_t *voxels, int axis, int forward, int right, int c, int forward_offset, int right_offset) {
  for (int i = 0; i < 8; i++) { // looping over ao_dirs
    if (solid_check(arr_at(voxels, get_axis_i(axis, right + ao_dirs[i][0], forward + ao_dirs[i][1], c))) !=
        solid_check(arr_at(voxels, get_axis_i(axis, right + right_offset + ao_dirs[i][0], forward + forward_offset + ao_dirs[i][1], c)))
    ) {
      return false;
    }
  }
  return true;
}

static inline const bool compare_forward(uint8_t *voxels, int axis, int forward, int right, int bit_pos, int light_dir) {
  return
    arr_at(voxels, get_axis_i(axis, right, forward, bit_pos)) == arr_at(voxels, get_axis_i(axis, right, forward + 1, bit_pos)) &&
    compare_ao(voxels, axis, forward, right, bit_pos + light_dir, 1, 0);
}

static inline const bool compare_right(uint8_t *voxels, int axis, int forward, int right, int bit_pos, int light_dir) {
  return
    arr_at(voxels, get_axis_i(axis, right, forward, bit_pos)) == arr_at(voxels, get_axis_i(axis, right + 1, forward, bit_pos)) &&
    compare_ao(voxels, axis, forward, right, bit_pos + light_dir, 0, 1);
}

static inline void insert_quad(arr_uint64_t* vertices, arr_uint32_t* indices, uint64_t v1, uint64_t v2, uint64_t v3, uint64_t v4, bool flipped) {
	int offset = vertices->length;

	uint64_t_arr_push(vertices, v1);
	uint64_t_arr_push(vertices, v2);
	uint64_t_arr_push(vertices, v3);
	uint64_t_arr_push(vertices, v4);

	if (flipped) {
		uint32_t_arr_push(indices, offset + 0);
		uint32_t_arr_push(indices, offset + 1);
		uint32_t_arr_push(indices, offset + 3);
		uint32_t_arr_push(indices, offset + 3);
		uint32_t_arr_push(indices, offset + 1);
		uint32_t_arr_push(indices, offset + 2);
  } else {
		uint32_t_arr_push(indices, offset + 0);
		uint32_t_arr_push(indices, offset + 1);
		uint32_t_arr_push(indices, offset + 2);
		uint32_t_arr_push(indices, offset + 2);
		uint32_t_arr_push(indices, offset + 3);
		uint32_t_arr_push(indices, offset + 0);
  	}
}

static inline const uint64_t get_vertex(uint64_t x, uint64_t y, uint64_t z, uint64_t type, uint64_t u, uint64_t v, uint64_t face, uint64_t ao, uint64_t opaque) {
	return (opaque << 63) | (ao << 61) | (face << 58) | (v << 50) | (type << 32) | (u << 24) | ((z - 1) << 16) | ((y - 1) << 8) | (x - 1);
}

struct MeshData mesh(uint8_t* voxels, bool opaque) {
	struct MeshData mesh_data;

	uint64_t axis_cols[CS_P2 * 3] = { 0 };
	uint64_t col_face_masks[CS_P2 * 6];

	for (int i = 0; i < 6; i++) {
		mesh_data.meshes[i].vertices = uint64_t_array();
		mesh_data.meshes[i].indices = uint32_t_array();
	}

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

          if (compare_forward(voxels, axis, forward, right, bit_pos, light_dir)) {
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
            compare_right(voxels, axis, forward, right, bit_pos, light_dir)
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

          uint8_t type = arr_at(voxels, get_axis_i(axis, right, forward, bit_pos));

          uint64_t v1, v2, v3, v4;
          if (face == 0) {
              v1 = get_vertex(mesh_left, mesh_up, mesh_front, type, 0, mesh_back - mesh_front, face, ao_LF, opaque);
              v2 = get_vertex(mesh_left, mesh_up, mesh_back, type, 0, 0, face, ao_LB, opaque);
              v3 = get_vertex(mesh_right, mesh_up, mesh_back, type, mesh_right - mesh_left, 0, face, ao_RB, opaque);
              v4 = get_vertex(mesh_right, mesh_up, mesh_front, type, mesh_right - mesh_left, mesh_back - mesh_front, face, ao_RF, opaque);
          } else if (face == 1) {
              v1 = get_vertex(mesh_left, mesh_up, mesh_back, type, 0, 0, face, ao_LB, opaque);
              v2 = get_vertex(mesh_left, mesh_up, mesh_front, type, 0, mesh_back - mesh_front, face, ao_LF, opaque);
              v3 = get_vertex(mesh_right, mesh_up, mesh_front, type, mesh_right - mesh_left, mesh_back - mesh_front, face, ao_RF, opaque);
              v4 = get_vertex(mesh_right, mesh_up, mesh_back, type, mesh_right - mesh_left, 0, face, ao_RB, opaque);
          } else if (face == 2) {
              v1 = get_vertex(mesh_up, mesh_front, mesh_left, type, mesh_right - mesh_left, 0, face, ao_LF, opaque);
              v2 = get_vertex(mesh_up, mesh_back, mesh_left, type, mesh_right - mesh_left, mesh_back - mesh_front, face, ao_LB, opaque);
              v3 = get_vertex(mesh_up, mesh_back, mesh_right, type, 0, mesh_back - mesh_front, face, ao_RB, opaque);
              v4 = get_vertex(mesh_up, mesh_front, mesh_right, type, 0, 0, face, ao_RF, opaque);
          } else if (face == 3) {
              v1 = get_vertex(mesh_up, mesh_back, mesh_left, type, 0, mesh_back - mesh_front, face, ao_LB, opaque);
              v2 = get_vertex(mesh_up, mesh_front, mesh_left, type, 0, 0, face, ao_LF, opaque);
              v3 = get_vertex(mesh_up, mesh_front, mesh_right, type, mesh_right - mesh_left, 0, face, ao_RF, opaque);
              v4 = get_vertex(mesh_up, mesh_back, mesh_right, type, mesh_right - mesh_left, mesh_back - mesh_front, face, ao_RB, opaque);
          } else if (face == 4) {
            v1 = get_vertex(mesh_front, mesh_left, mesh_up, type, 0, 0, face, ao_LF, opaque);
            v2 = get_vertex(mesh_back, mesh_left, mesh_up, type, mesh_back - mesh_front, 0, face, ao_LB, opaque);
            v3 = get_vertex(mesh_back, mesh_right, mesh_up, type, mesh_back - mesh_front, mesh_right - mesh_left, face, ao_RB, opaque);
            v4 = get_vertex(mesh_front, mesh_right, mesh_up, type, 0, mesh_right - mesh_left, face, ao_RF, opaque);
          } else if (face == 5) {
            v1 = get_vertex(mesh_back, mesh_left, mesh_up, type, 0, 0, face, ao_LB, opaque);
            v2 = get_vertex(mesh_front, mesh_left, mesh_up, type, mesh_back - mesh_front, 0, face, ao_LF, opaque);
            v3 = get_vertex(mesh_front, mesh_right, mesh_up, type, mesh_back - mesh_front, mesh_right - mesh_left, face, ao_RF, opaque);
            v4 = get_vertex(mesh_back, mesh_right, mesh_up, type, 0, mesh_right - mesh_left, face, ao_RB, opaque);
          }

          insert_quad(&mesh_data.meshes[face].vertices, &mesh_data.meshes[face].indices, v1, v2, v3, v4, ao_LB + ao_RF > ao_RB + ao_LF);
        }
      }
    }
  }

  return mesh_data;
}
#include "../../world/chunk.h"
#include "../../world/block.h"
#include "../../world/world.h"
#include "raycast.h"

// Blocks that can be edited range from 1 to CHUNK_SIZE

struct Ray ray_cast(vec3 startPosition, vec3 rayDirection, float maxDistance) {
    struct Ray ray;

    ray.blockFound = false;
    ray.maxDistance = maxDistance;

    const vec3 rayUnitStepSize = {
        sqrt(1 + (rayDirection[1] / rayDirection[0]) * (rayDirection[1] / rayDirection[0]) + (rayDirection[2] / rayDirection[0]) * (rayDirection[2] / rayDirection[0])),
        sqrt(1 + (rayDirection[0] / rayDirection[1]) * (rayDirection[0] / rayDirection[1]) + (rayDirection[2] / rayDirection[1]) * (rayDirection[2] / rayDirection[1])),
        sqrt(1 + (rayDirection[0] / rayDirection[2]) * (rayDirection[0] / rayDirection[2]) + (rayDirection[1] / rayDirection[2]) * (rayDirection[1] / rayDirection[2]))
    };

    ivec3 check = {floor(startPosition[0]), floor(startPosition[1]), floor(startPosition[2])};
    vec3 rayLength1D;
    ivec3 step;

    // Initialize step and initial ray length for each axis
    for (int i = 0; i < 3; i++) {
        if (rayDirection[i] < 0) {
            step[i] = -1;
            rayLength1D[i] = (startPosition[i] - check[i]) * rayUnitStepSize[i];
        } else {
            step[i] = 1;
            rayLength1D[i] = ((check[i] + 1) - startPosition[i]) * rayUnitStepSize[i];
        }
    }

    float currentDistance = 0.0f;
    while (!ray.blockFound && currentDistance < ray.maxDistance) {
        const int axis = (rayLength1D[0] < rayLength1D[1]) ? ((rayLength1D[0] < rayLength1D[2]) ? 0 : 2) : ((rayLength1D[1] < rayLength1D[2]) ? 1 : 2);

        check[axis] += step[axis];
        currentDistance = rayLength1D[axis];
        rayLength1D[axis] += rayUnitStepSize[axis];

        glm_ivec3_copy(check, ray.worldPosition);

        ivec3 chunkPos;
        glm_ivec3_copy((ivec3) {
            check[0] / CHUNK_SIZE,
            check[1] / CHUNK_SIZE,
            check[2] / CHUNK_SIZE},
        chunkPos);

        // +1 to account for voxel padding
        ivec3 blockPos;
        glm_ivec3_copy((ivec3) {
            (check[0] % CHUNK_SIZE) + 1,
            (check[1] % CHUNK_SIZE) + 1,
            (check[2] % CHUNK_SIZE) + 1},
        blockPos);

        for (int i = 0; i < 3; i++) {
            if (blockPos[i] < 1) {
                blockPos[i] += CHUNK_SIZE;
                chunkPos[i] -= 1;
            }
        }

        struct Chunk *chunkToModify = world_getChunk(chunkPos);

        if (chunkToModify == NULL) {
            break;
        }

        if (chunkToModify->voxels[blockIndex(blockPos[0], blockPos[1], blockPos[2])] != BLOCK_AIR) {
            if (axis == 0) {
                if (rayDirection[axis] > 0) ray.placedDirection = LEFT;
                else ray.placedDirection = RIGHT;
            } else if (axis == 1) {
                if (rayDirection[axis] > 0) ray.placedDirection = BOTTOM;
                else ray.placedDirection = TOP;
            } else if (axis == 2) {
                if (rayDirection[axis] > 0) ray.placedDirection = FRONT;
                else ray.placedDirection = BACK;
            }

            glm_ivec3_copy((ivec3){blockPos[0], blockPos[1], blockPos[2]}, ray.blockFoundPosition);
            ray.chunkToModify = chunkToModify;
            ray.blockFound = true;
        }
    }

    return ray;
}

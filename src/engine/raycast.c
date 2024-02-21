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

        ivec3 chunkPos = (ivec3){
            check[0] / 32,
            check[1] / 32,
            check[2] / 32
        };

        ivec3 blockPos = (ivec3){
            (check[0] % 32) + 1,
            (check[1] % 32) + 1,
            (check[2] % 32) + 1
        };

        ray.chunkToModify = world_getChunk(chunkPos);

        if (ray.chunkToModify->voxels[blockIndex(blockPos[0], blockPos[1], blockPos[2])] != BLOCK_AIR) {
            glm_ivec3_copy(blockPos, ray.blockFoundPosition);

            printf("CHUNK [%d %d %d] BLOCK: [%d %d %d]\n", chunkPos[0], chunkPos[1], chunkPos[2], blockPos[0], blockPos[1], blockPos[2]);

            ray.blockFound = true;
        }
    }

    return ray;
}

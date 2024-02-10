#include "raycast.h"

struct Ray ray_cast(vec3 startPosition, vec3 rayDirection) {
    struct Ray ray;

    ray.blockFound = false;
    ray.maxDistance = 128.0f;

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
            rayLength1D[i] = (check[i] + 1 - startPosition[i]) * rayUnitStepSize[i];
        }
    }

    float currentDistance = 0.0f;
    while (!ray.blockFound && currentDistance < ray.maxDistance)  {
        const int axis = (rayLength1D[0] < rayLength1D[1]) ? ((rayLength1D[0] < rayLength1D[2]) ? 0 : 2) : ((rayLength1D[1] < rayLength1D[2]) ? 1 : 2);

        check[axis] += step[axis];
        currentDistance = rayLength1D[axis];
        rayLength1D[axis] += rayUnitStepSize[axis];

        ivec3 chunkPos = (ivec3) {
            floor(check[0] / 16.0f),
            check[1],
            floor(check[2] / 16.0f)
        };

        ivec3 blockPos = (ivec3) {
            check[0] % 16,
            check[1],
            check[2] % 16
        };

        if (blockPos[0] < 0) blockPos[0] += 16;
        if (blockPos[2] < 0) blockPos[2] += 16;


        struct Chunk *chunkToModify = world_getChunk(world_hashChunk((ivec2s){chunkPos[0], chunkPos[2]}));

        //printf("CHECK: %d %d %d CHUNK: %d %d\n", check[0], check[1], check[2], chunkPos.x, chunkPos.y);

        if (blockPos[1] > 0) {
            if (chunkToModify->blocks[blockIndex(blockPos[0], blockPos[1], blockPos[2])].id != BLOCK_AIR) {
                glm_vec3_copy((vec3){blockPos[0], blockPos[1], blockPos[2]}, ray.blockFoundPosition);
                ray.chunkToModify = chunkToModify;
                ray.blockFound = true;
            }
        }
    }

    return ray;
}

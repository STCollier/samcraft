#include "raycast.h"

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
            rayLength1D[i] = (check[i] + 1 - startPosition[i]) * rayUnitStepSize[i];
        }
    }

    float currentDistance = 0.0f;
    while (!ray.blockFound && currentDistance < ray.maxDistance)  {
        const int axis = (rayLength1D[0] < rayLength1D[1]) ? ((rayLength1D[0] < rayLength1D[2]) ? 0 : 2) : ((rayLength1D[1] < rayLength1D[2]) ? 1 : 2);

        check[axis] += step[axis];
        currentDistance = rayLength1D[axis];
        rayLength1D[axis] += rayUnitStepSize[axis];

        float size = CHUNK_SIZE; // This needs to be float division to work with floor()

        ivec3 chunkPos;
        glm_ivec3_copy((ivec3) {
            floor(check[0] / size),
            check[1],
            floor(check[2] / size)},
        chunkPos);

        ivec3 blockPos;
        glm_ivec3_copy((ivec3) {
            check[0] % CHUNK_SIZE,
            check[1],
            check[2] % CHUNK_SIZE}, 
        blockPos);

        if (blockPos[0] < 0) blockPos[0] += CHUNK_SIZE;
        if (blockPos[2] < 0) blockPos[2] += CHUNK_SIZE;


        struct Chunk *chunkToModify = world_getChunk(world_hashChunk((ivec2s){chunkPos[0], chunkPos[2]}));

        if (blockPos[1] >= 0) {
            if (chunkToModify->blocks[blockIndex(blockPos[0], blockPos[1], blockPos[2])].id != BLOCK_AIR) {
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
    }

    return ray;
}

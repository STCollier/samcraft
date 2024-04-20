#include "player.h"
#include "globals.h"
#include "block_overlay.h"
#include "debugblock.h"

struct Player player;

void reflect(vec3 velocity, ivec3 normal, vec3 out) {
    const float dot = velocity[0] * normal[0] + velocity[1] * normal[1] + velocity[2] * normal[2];
    const float ux = normal[0] * dot;
    const float uy = normal[1] * dot;
    const float uz = normal[2] * dot;
    const float wx = velocity[0] - ux;
    const float wy = velocity[1] - uy;
    const float wz = velocity[2] - uz;
    out[0] = wx - ux;
    out[1] = wy - uy;
    out[2] = wz - uz;
}


static bool player_collision(ivec3 position) {
    return (
        camera.position[0] - player.dimensions[0] <= position[0] + 1 &&
        camera.position[0] + player.dimensions[0] >= position[0] &&
        camera.position[1] - player.dimensions[1] <= position[1] + 1 &&
        camera.position[1] + player.dimensions[1] >= position[1] &&
        camera.position[2] - player.dimensions[2] <= position[2] + 1 &&
        camera.position[2] + player.dimensions[2] >= position[2]
    );
}

struct Chunk* getChunkFromPosition(int x, int y, int z) {
    ivec3 blockPosition, chunkPosition;
    glm_ivec3_copy((ivec3){x, y, z}, blockPosition);

    glm_ivec3_copy((ivec3){blockPosition[0] / CHUNK_SIZE, blockPosition[1] / CHUNK_SIZE, blockPosition[2] / CHUNK_SIZE}, chunkPosition);
    for (int i = 0; i < 3; i++) {
        if (blockPosition[i] < 0) {
            chunkPosition[i] -= 1;
        }
    }

    return world_getChunk(chunkPosition);
}

int getBlockIndexFromWorldPosition(int x, int y, int z) {
    ivec3 worldPosition, blockPosition;
    glm_ivec3_copy((ivec3){x, y, z}, worldPosition);
    glm_ivec3_copy((ivec3) {
        (worldPosition[0] % CHUNK_SIZE) + 1,
        (worldPosition[1] % CHUNK_SIZE) + 1,
        (worldPosition[2] % CHUNK_SIZE) + 1},
    blockPosition);

    for (int i = 0; i < 3; i++) {
        if (blockPosition[i] < 1) {
            blockPosition[i] += CHUNK_SIZE;
        }
    }

    return blockIndex(blockPosition[0], blockPosition[1], blockPosition[2]);
}

void player_init() {
    player.FOV = globals.FOV;
    glm_vec3_copy((vec3){25.0f, 125.0f, 25.0f}, camera.position);
    glm_vec3_copy((vec3){0.6, 1.8, 0.6}, player.dimensions);
    glm_ivec3_copy((ivec3){camera.position[0] / CHUNK_SIZE, camera.position[1] / CHUNK_SIZE, camera.position[2] / CHUNK_SIZE}, player.chunkPosition);
    glm_ivec3_copy((ivec3){0, 0, 0}, player.previousPosition);

    glm_vec3_copy(camera.position, player.aabb.position);
    glm_vec3_copy((vec3){player.dimensions[0] / 2, player.dimensions[1] / 2, player.dimensions[2] / 2}, player.aabb.half);

    camera_init(player.FOV, globals.mouseSensitivity, camera.position);
    player.exitedChunk = false;

    player.breakTime = 0.0f;
    player.selectedBlock = block_getID("dirt");
    player.reach = globals.reach;
}

struct AABB colliders[512];
size_t numColliders = 0;

void player_update() {
    glm_vec3_copy(camera.position, camera.position);
    glm_ivec3_copy((ivec3){camera.position[0] / CHUNK_SIZE, camera.position[1] / CHUNK_SIZE, camera.position[2] / CHUNK_SIZE}, player.chunkPosition);
    for (int i = 0; i < 3; i++) {
        if (camera.position[i] < 0) {
            player.chunkPosition[i] -= 1;
        }
    }

    for (int z = floor(camera.position[2] - 2); z <= ceil(camera.position[2] + 2); z++) {
        for (int y = floor(camera.position[1] - 3); y <= ceil(camera.position[1] + 3); y++) {
            for (int x = floor(camera.position[0] - 2); x <= ceil(camera.position[0] + 2); x++) {
                struct Chunk* chunk = getChunkFromPosition(x, y, z);

                if (chunk->voxels[getBlockIndexFromWorldPosition(x, y, z)] != BLOCK_AIR) {
                    struct AABB voxelAABB = {
                        .position = {x + 1.0, y + 1.0, z + 1.0},
                        .half = {0.5, 0.5, 0.5}
                    };

                    colliders[numColliders] = voxelAABB;
                    numColliders++;
                }
            }
        }
    }

    for (int i = 0; i < 3; i++) {
        struct Sweep sweep = sweepInto(player.aabb, colliders, numColliders, camera.motion);
        if (sweep.hit.result) {
            //puts("hit!");
            vec3 v;
            float p = glm_vec3_dot(camera.motion, (vec3){sweep.hit.normal[0], sweep.hit.normal[1], sweep.hit.normal[2]});

            for (int i = 0; i < 3; i++) {
                if (sweep.hit.normal[i] == 1)
                    glm_vec3_sub(camera.motion, (vec3){p, p, p}, v);
                else if (sweep.hit.normal[i] == -1)
                    glm_vec3_add(camera.motion, (vec3){p, p, p}, v);
            }

            //camera.speed = 10;
            for (int i = 0; i < 3; i++) {
                if (sweep.hit.normal[i] != 0) {
                    camera.motion[i] = v[i];
                }
            }
        }
    }

    //printf("%zu\n", numColliders);

    numColliders = 0;

    glm_vec3_add(camera.position, camera.motion, camera.position);
    //printf("%f %f %f\n", camera.position[0], camera.position[1], camera.position[2]);

    glm_vec3_copy((vec3){
        camera.position[0] + player.dimensions[0],
        camera.position[1] - player.aabb.half[1]/2,
        camera.position[2] + player.dimensions[2],
    }, player.aabb.position);

    if ((player.chunkPosition[0] != player.previousPosition[0]) || 
        (player.chunkPosition[1] != player.previousPosition[1]) ||
        (player.chunkPosition[2] != player.previousPosition[2])) {

        glm_ivec3_copy(player.chunkPosition, player.previousPosition);
        player.exitedChunk = true;
    }

    if (window.keyPressed[GLFW_KEY_1]) {
        player.selectedBlock = block_getID("dirt"); 
    } else if (window.keyPressed[GLFW_KEY_2]) {
        player.selectedBlock = block_getID("grass");
    } else if (window.keyPressed[GLFW_KEY_3]) {
        player.selectedBlock = block_getID("planks");
    } else if (window.keyPressed[GLFW_KEY_4]) {
        player.selectedBlock = block_getID("log");
    } else if (window.keyPressed[GLFW_KEY_5]) {
        player.selectedBlock = block_getID("stone");
    } else if (window.keyPressed[GLFW_KEY_6]) {
        player.selectedBlock = block_getID("sand");
    } else if (window.keyPressed[GLFW_KEY_7]) {
        player.selectedBlock = block_getID("cobblestone");
    } else if (window.keyPressed[GLFW_KEY_8]) {
        player.selectedBlock = block_getID("S");
    } else if (window.keyPressed[GLFW_KEY_9]) {
        player.selectedBlock = block_getID("A");
    } else if (window.keyPressed[GLFW_KEY_0]) {
        player.selectedBlock = block_getID("M");
    }
}

void player_raycast() {
    player.ray = ray_cast(camera.position, camera.direction, player.reach);
}

void player_placeBlock() {
    struct Chunk *chunkToModify = player.ray.chunkToModify;

    if (player.ray.blockFound) {
        ivec3 blockPlaceLocation;

        switch (player.ray.placedDirection) {
            case FRONT:
                glm_ivec3_copy((ivec3){player.ray.blockFoundPosition[0], player.ray.blockFoundPosition[1], player.ray.blockFoundPosition[2] - 1}, blockPlaceLocation);
                break;
            case BACK:
                glm_ivec3_copy((ivec3){player.ray.blockFoundPosition[0], player.ray.blockFoundPosition[1], player.ray.blockFoundPosition[2] + 1}, blockPlaceLocation);
                break;
            case BOTTOM:
                glm_ivec3_copy((ivec3){player.ray.blockFoundPosition[0], player.ray.blockFoundPosition[1] - 1, player.ray.blockFoundPosition[2]}, blockPlaceLocation);
                break;
            case TOP:
                glm_ivec3_copy((ivec3){player.ray.blockFoundPosition[0], player.ray.blockFoundPosition[1] + 1, player.ray.blockFoundPosition[2]}, blockPlaceLocation);
                break;
            case RIGHT:
                glm_ivec3_copy((ivec3){player.ray.blockFoundPosition[0] + 1, player.ray.blockFoundPosition[1], player.ray.blockFoundPosition[2]}, blockPlaceLocation);
                break;
            case LEFT:
                glm_ivec3_copy((ivec3){player.ray.blockFoundPosition[0] - 1, player.ray.blockFoundPosition[1], player.ray.blockFoundPosition[2]}, blockPlaceLocation);
                break;
        }

        //printf("PLACE: [C: %d %d %d] [B: %d %d %d]\n", chunkToModify->position[0], chunkToModify->position[1], chunkToModify->position[2], blockPlaceLocation[0], blockPlaceLocation[1], blockPlaceLocation[2]);

        if (blockPlaceLocation[0] == CHUNK_SIZE + 1) {
            glm_ivec3_copy((ivec3){1, player.ray.blockFoundPosition[1], player.ray.blockFoundPosition[2]}, blockPlaceLocation);
            chunkToModify = world_getChunk((ivec3){chunkToModify->position[0] + 1, chunkToModify->position[1], chunkToModify->position[2]});
        } else if (blockPlaceLocation[0] == 0) {
            glm_ivec3_copy((ivec3){CHUNK_SIZE, player.ray.blockFoundPosition[1], player.ray.blockFoundPosition[2]}, blockPlaceLocation);
            chunkToModify = world_getChunk((ivec3){chunkToModify->position[0] - 1, chunkToModify->position[1], chunkToModify->position[2]});
        }

        if (blockPlaceLocation[1] == CHUNK_SIZE + 1) {
            glm_ivec3_copy((ivec3){player.ray.blockFoundPosition[0], 1, player.ray.blockFoundPosition[2]}, blockPlaceLocation);
            chunkToModify = world_getChunk((ivec3){chunkToModify->position[0], chunkToModify->position[1] + 1, chunkToModify->position[2]});
        } else if (blockPlaceLocation[1] == 0) {
            glm_ivec3_copy((ivec3){player.ray.blockFoundPosition[0], CHUNK_SIZE, player.ray.blockFoundPosition[2]}, blockPlaceLocation);
            chunkToModify = world_getChunk((ivec3){chunkToModify->position[0], chunkToModify->position[1] - 1, chunkToModify->position[2]});
        }

        if (blockPlaceLocation[2] == CHUNK_SIZE + 1) {
            glm_ivec3_copy((ivec3){player.ray.blockFoundPosition[0], player.ray.blockFoundPosition[1], 1}, blockPlaceLocation);
            chunkToModify = world_getChunk((ivec3){chunkToModify->position[0], chunkToModify->position[1], chunkToModify->position[2] + 1});
        } else if (blockPlaceLocation[2] == 0) {
            glm_ivec3_copy((ivec3){player.ray.blockFoundPosition[0], player.ray.blockFoundPosition[1], CHUNK_SIZE}, blockPlaceLocation);
            chunkToModify = world_getChunk((ivec3){chunkToModify->position[0], chunkToModify->position[1], chunkToModify->position[2] - 1});
        }

        //printf("WHERE TO PLACE: [C: %d %d %d] [B: %d %d %d]\n\n", chunkToModify->position[0], chunkToModify->position[1], chunkToModify->position[2], blockPlaceLocation[0], blockPlaceLocation[1], blockPlaceLocation[2]);

        // This is where we actually edit the chunk
        chunkToModify->voxels[blockIndex(blockPlaceLocation[0], blockPlaceLocation[1], blockPlaceLocation[2])] = player.selectedBlock;

        // X axis remeshing
        if (blockPlaceLocation[0] == 1) {
            struct Chunk *c = world_getChunk((ivec3){chunkToModify->position[0] - 1, chunkToModify->position[1], chunkToModify->position[2]});
            world_remeshChunk(c->position);
        } else if (blockPlaceLocation[0] == CHUNK_SIZE) {
            struct Chunk *c = world_getChunk((ivec3){chunkToModify->position[0] + 1, chunkToModify->position[1], chunkToModify->position[2]});
            world_remeshChunk(c->position);
        }

        // Y axis remeshing
        if (player.ray.blockFoundPosition[1] == 1) {
            struct Chunk *c = world_getChunk((ivec3){chunkToModify->position[0], chunkToModify->position[1] - 1, chunkToModify->position[2]});
            world_remeshChunk(c->position);
        } else if (player.ray.blockFoundPosition[1] == CHUNK_SIZE) {
            struct Chunk *c = world_getChunk((ivec3){chunkToModify->position[0], chunkToModify->position[1] + 1, chunkToModify->position[2]});
            world_remeshChunk(c->position);
        }

        // Z axis Remeshing
        if (blockPlaceLocation[2] == 1) {
            struct Chunk *c = world_getChunk((ivec3){chunkToModify->position[0], chunkToModify->position[1], chunkToModify->position[2] - 1});
            world_remeshChunk(c->position);
        } else if (blockPlaceLocation[2] == CHUNK_SIZE) {
            struct Chunk *c = world_getChunk((ivec3){chunkToModify->position[0], chunkToModify->position[1], chunkToModify->position[2] + 1});
            world_remeshChunk(c->position);
        }

        // Remesh to fix AO on diagonal chunks 
        if (blockPlaceLocation[0] == CHUNK_SIZE && blockPlaceLocation[2] == CHUNK_SIZE) {
            struct Chunk *c = world_getChunk((ivec3){chunkToModify->position[0] + 1, chunkToModify->position[1], chunkToModify->position[2] + 1});
            world_remeshChunk(c->position);
        } else if (blockPlaceLocation[0] == 1 && blockPlaceLocation[2] == 1) {
            struct Chunk *c = world_getChunk((ivec3){chunkToModify->position[0] - 1, chunkToModify->position[1], chunkToModify->position[2] - 1});
            world_remeshChunk(c->position);
        } else if (blockPlaceLocation[0] == 1 && blockPlaceLocation[2] == CHUNK_SIZE) {
            struct Chunk *c = world_getChunk((ivec3){chunkToModify->position[0] - 1, chunkToModify->position[1], chunkToModify->position[2] + 1});
            world_remeshChunk(c->position);
        } else if (blockPlaceLocation[0] == CHUNK_SIZE && blockPlaceLocation[2] == 1) {
            struct Chunk *c = world_getChunk((ivec3){chunkToModify->position[0] + 1, chunkToModify->position[1], chunkToModify->position[2] - 1});
            world_remeshChunk(c->position);
        }

        world_remeshChunk(chunkToModify->position);
    }
}

void player_destroyBlock(shader_t blockOverlayShader) {
    struct Chunk *chunkToModify = player.ray.chunkToModify;

    if (player.ray.blockFound) {
        float hardness = block_getHardnessValue(chunkToModify->voxels[blockIndex(player.ray.blockFoundPosition[0], player.ray.blockFoundPosition[1], player.ray.blockFoundPosition[2])]);

        if (player.breakTime == 0) glm_ivec3_copy(player.ray.worldPosition, player.previousRayLookAt);

        if (player.previousRayLookAt[0] == player.ray.worldPosition[0] && player.previousRayLookAt[1] == player.ray.worldPosition[1] && player.previousRayLookAt[2] == player.ray.worldPosition[2]) {
            player.breakTime += window.dt;

            if (player.ray.blockFound) block_overlay_use(blockOverlayShader, (int) ((player.breakTime / hardness) * 5.0));
        } else {
            player.breakTime = 0;
        }

        if (player.breakTime >= hardness) {
            chunkToModify->voxels[blockIndex(player.ray.blockFoundPosition[0], player.ray.blockFoundPosition[1], player.ray.blockFoundPosition[2])] = BLOCK_AIR;
            ivec3 newPosition;

            // X axis remeshing
            if (player.ray.blockFoundPosition[0] == 1) {
                glm_ivec3_copy((ivec3){chunkToModify->position[0] - 1, chunkToModify->position[1], chunkToModify->position[2]}, newPosition);
                world_remeshChunk(newPosition);
            } else if (player.ray.blockFoundPosition[0] == CHUNK_SIZE) {
                glm_ivec3_copy((ivec3){chunkToModify->position[0] + 1, chunkToModify->position[1], chunkToModify->position[2]}, newPosition);
                world_remeshChunk(newPosition);
            }

            // Y axis remeshing
            if (player.ray.blockFoundPosition[1] == 1) {
                glm_ivec3_copy((ivec3){chunkToModify->position[0], chunkToModify->position[1] - 1, chunkToModify->position[2]}, newPosition);
                world_remeshChunk(newPosition);
            } else if (player.ray.blockFoundPosition[1] == CHUNK_SIZE) {
                glm_ivec3_copy((ivec3){chunkToModify->position[0], chunkToModify->position[1] + 1, chunkToModify->position[2]}, newPosition);
                world_remeshChunk(newPosition);
            }

            // Z axis remeshing
            if (player.ray.blockFoundPosition[2] == 1) {
                glm_ivec3_copy((ivec3){chunkToModify->position[0], chunkToModify->position[1], chunkToModify->position[2] - 1}, newPosition);
                world_remeshChunk(newPosition);
            } else if (player.ray.blockFoundPosition[2] == CHUNK_SIZE) {
                glm_ivec3_copy((ivec3){chunkToModify->position[0], chunkToModify->position[1], chunkToModify->position[2] + 1}, newPosition);
                world_remeshChunk(newPosition);
            }

            // Remesh to fix AO on diagonal chunks 
            if (player.ray.blockFoundPosition[0] == CHUNK_SIZE && player.ray.blockFoundPosition[2] == CHUNK_SIZE) {
                glm_ivec3_copy((ivec3){chunkToModify->position[0] + 1, chunkToModify->position[1], chunkToModify->position[2] + 1}, newPosition);
                world_remeshChunk(newPosition);
            } else if (player.ray.blockFoundPosition[0] == 1 && player.ray.blockFoundPosition[2] == 1) {
                glm_ivec3_copy((ivec3){chunkToModify->position[0] - 1, chunkToModify->position[1], chunkToModify->position[2] - 1}, newPosition);
                world_remeshChunk(newPosition);
            } else if (player.ray.blockFoundPosition[0] == 1 && player.ray.blockFoundPosition[2] == CHUNK_SIZE) {
                glm_ivec3_copy((ivec3){chunkToModify->position[0] - 1, chunkToModify->position[1], chunkToModify->position[2] + 1}, newPosition);
                world_remeshChunk(newPosition);
            } else if (player.ray.blockFoundPosition[0] == CHUNK_SIZE && player.ray.blockFoundPosition[2] == 1) {
                glm_ivec3_copy((ivec3){chunkToModify->position[0] + 1, chunkToModify->position[1], chunkToModify->position[2] - 1}, newPosition);
                world_remeshChunk(newPosition);
            }

            world_remeshChunk(chunkToModify->position);

            player.breakTime = 0;
        }
    }
}
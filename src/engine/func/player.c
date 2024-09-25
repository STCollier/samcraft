#include "../core/window.h"
#include "../core/camera.h"
#include "../core/shader.h"
#include "../core/globals.h"
#include "../util/lerp.h"
#include "../gfx/block-overlay.h"
#include "../../world/world.h"
#include "../../world/chunk.h"
#include "../../world/block.h"
#include "raycast.h"
#include "physics.h"
#include "player.h"

struct Player player;

const float ACCELERATION = 0.2;
const float FRICTION = 0.5;
const float GRAVITY = 9.81;
const float JUMP_HEIGHT = 6.0;

void player_init() {
    player.FOV = globals.FOV;
    player.currentFOV = globals.FOV;

    glm_vec3_copy((vec3){55.0f, 200.0f, 55.0f}, camera.position);
    glm_vec3_copy((vec3){0.6, 1.8, 0.6}, player.dimensions);
    glm_ivec3_copy((ivec3){camera.position[0] / CHUNK_SIZE, camera.position[1] / CHUNK_SIZE, camera.position[2] / CHUNK_SIZE}, player.chunkPosition);
    glm_ivec3_copy((ivec3){0, 0, 0}, player.previousPosition);

    glm_vec3_copy(camera.position, player.aabb.position);
    glm_vec3_copy((vec3){player.dimensions[0] / 2, player.dimensions[1] / 2, player.dimensions[2] / 2}, player.aabb.half);

    camera_init(player.FOV, globals.mouseSensitivity, camera.position);
    player.exitedChunk = false;

    player.cameraFOVLerp = (lerp_t) {
        .type = EASE_IN_QUAD,
        .values = {
            .from = player.currentFOV,
            .to = player.currentFOV + 10
        },
        .time = 0
    };

    player.breakTime = 0.0f;
    player.selectedBlock = block_getID("dirt");
    player.reach = globals.reach;

    block_overlay_bind();
}

struct AABB colliders[512];
size_t numColliders = 0;
bool clicked = false;
bool toggledSneak = false;

void player_update(shader_t blockOverlayShader) {
    player.currentFOV = globals.FOV;

    glm_vec3_copy((vec3){
        camera.position[0] - player.aabb.half[0],
        camera.position[1] - player.dimensions[1], // Raise the player's view higher up (not centered unlike the x and z axis)
        camera.position[2] - player.aabb.half[2],
    }, player.aabb.position);

    glm_ivec3_copy((ivec3){
        camera.position[0] / CHUNK_SIZE,
        camera.position[1] / CHUNK_SIZE,
        camera.position[2] / CHUNK_SIZE
    }, player.chunkPosition);

    for (int i = 0; i < 3; i++) {
        if (camera.position[i] < 0) {
            player.chunkPosition[i] -= 1;
        }
    }

    if (ivec3_nequal(player.chunkPosition, player.previousPosition)) {
        glm_ivec3_copy(player.chunkPosition, player.previousPosition);
        player.exitedChunk = true;
    }

    if (vec3_nequal(camera.tempPosition, camera.position)) {
        glm_vec3_copy(camera.tempPosition, camera.oldPosition);
        glm_vec3_copy(camera.position, camera.tempPosition);

        vec3 delta;
        glm_vec3_sub(camera.position, camera.oldPosition, delta);
        glm_vec3_copy((vec3){fabsf(delta[0]), fabsf(delta[1]), fabsf(delta[2])}, camera.delta);
        
    }

    //printf("%f\n", camera.speed);

    // Collision detection for range of blocks around player
    for (int z = floor(camera.position[2] - 2); z <= ceil(camera.position[2] + 2); z++) {
        for (int y = floor(camera.position[1] - 3); y <= ceil(camera.position[1] + 4); y++) {
            for (int x = floor(camera.position[0] - 2); x <= ceil(camera.position[0] + 2); x++) {
                if (getBlockFromWorldPosition(x, y, z) != BLOCK_AIR) {
                    struct AABB voxelAABB = {
                        .position = {
                           x - player.aabb.half[0] + 0.5,
                           y - player.aabb.half[1] + 0.5,
                           z - player.aabb.half[2] + 0.5,
                        },
                        .half = {0.5, 0.5, 0.5}
                    };

                    colliders[numColliders] = voxelAABB;
                    numColliders++;
                }
            }
        }
    }

    // Collision resolution, it loops 3 times because the player can collide with at most 3 blocks at a time
    for (int i = 0; i < 3; i++) {
        struct Sweep sweep = sweepInto(player.aabb, colliders, numColliders, camera.motion);
        if (sweep.hit.result) {
            float p = glm_vec3_dot(camera.motion, (vec3){sweep.hit.normal[0], sweep.hit.normal[1], sweep.hit.normal[2]});
            vec3 v;

            for (int axis = 0; axis < 3; axis++) {
                if (sweep.hit.normal[axis] == 1)
                    glm_vec3_sub(camera.motion, (vec3){p, p, p}, v);
                else if (sweep.hit.normal[axis] == -1)
                    glm_vec3_add(camera.motion, (vec3){p, p, p}, v);
            }

            for (int axis = 0; axis < 3; axis++) {
                if (sweep.hit.normal[axis] != 0) {
                    if (axis == AXIS_Y) {
                        if (sweep.hit.normal[1] == 1) {
                            camera.velocity[1] = -GRAVITY;
                            camera.velocity[1] = clamp(camera.velocity[1], -GRAVITY, camera.velocity[1]);
                        } else {
                            camera.velocity[1] = 0;
                        }
                    } else {
                        if (player.state != SNEAK) player.state = WALK;
                    }
                    camera.motion[axis] = v[axis];
                }
            }
        }
    }

    if (player.state == SPRINT) {
        camera.speed = camera.speedValue[SPRINT];
        camera.fov = lerpTo(&player.cameraFOVLerp, 2.0 * window.dt);
    } else if (player.state == SNEAK) {
        camera.speed = camera.speedValue[SNEAK];
        camera.fov = lerpFrom(&player.cameraFOVLerp, 3.5 * window.dt);

        player.dimensions[1] = 1.5;
        player.aabb.half[1] = player.dimensions[1] / 2;

        toggledSneak = true;
    } else {
        camera.speed = camera.speedValue[WALK];
        camera.fov = lerpFrom(&player.cameraFOVLerp, 3.5 * window.dt);
    }

    if (toggledSneak && player.state != SNEAK) {
        player.dimensions[1] = 1.8;
        player.aabb.half[1] = player.dimensions[1] / 2;
        camera.position[1] += 0.3;
        toggledSneak = false;
    }

    numColliders = 0;

    glm_vec3_add(camera.position, camera.motion, camera.position);

    vec3 mx, my, mz;

    if (glfwGetKey(window.self, GLFW_KEY_S) == GLFW_PRESS && camera.velocity[2] > -camera.speed) {
        camera.acceleration[2] = -ACCELERATION;
    } else if (glfwGetKey(window.self, GLFW_KEY_W) == GLFW_PRESS && camera.velocity[2] < camera.speed) {
        camera.acceleration[2] = ACCELERATION;
    } else if (fabsf(camera.velocity[2]) > 1.0) {
        camera.acceleration[2] = (camera.velocity[2] < 0) ? FRICTION : -FRICTION;
    } else {
        camera.velocity[2] = 0;
        camera.acceleration[2] = 0;
    }

    vec3 horizontal = {camera.front[0], 0, camera.front[2]};
    camera.velocity[2] += camera.acceleration[2];
    glm_vec3_normalize(horizontal);
    glm_vec3_scale(horizontal, camera.velocity[2] * window.dt, mz);


   if (glfwGetKey(window.self, GLFW_KEY_A) == GLFW_PRESS && camera.velocity[0] > -camera.speed) {
        camera.acceleration[0] = -ACCELERATION;
    } else if (glfwGetKey(window.self, GLFW_KEY_D) == GLFW_PRESS && camera.velocity[0] < camera.speed) {
        camera.acceleration[0] = ACCELERATION;
    } else if (fabsf(camera.velocity[0]) > 1.0) {
        camera.acceleration[0] = (camera.velocity[0] < 0) ? FRICTION : -FRICTION;
    } else {
        camera.velocity[0] = 0;
        camera.acceleration[0] = 0;
    }

    camera.velocity[0] += camera.acceleration[0];
    glm_vec3_scale(camera.right, camera.velocity[0] * window.dt, mx);

    glm_vec3_add(mx, mz, camera.motion);


    if (glfwGetKey(window.self, GLFW_KEY_SPACE) == GLFW_PRESS && camera.velocity[1] == -GRAVITY) {
        camera.velocity[1] = JUMP_HEIGHT;
    }

    //printf("%f\n", camera.velocity[1]);

    camera.motion[1] += camera.velocity[1] * window.dt;
    camera.velocity[1] -= GRAVITY * window.dt;

    if (glfwGetKey(window.self, GLFW_KEY_E) == GLFW_PRESS) {
        player.state = SPRINT;
    } else if (glfwGetKey(window.self, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        player.state = SNEAK;
    } else {
        player.state = WALK;
    }

    //printf("%f\n", camera.motion[2]);

    if (window.leftClicked) {
        player_raycast();
        player_destroyBlock(blockOverlayShader);
    } else {
        player.breakTime = 0;
    }

    if (window.rightClicked && !clicked) {
        player_raycast();

        struct AABB voxel = {
            .position = {
                player.ray.worldPosition[0] - player.aabb.half[0] + 0.5,
                player.ray.worldPosition[1] + 0.5,
                player.ray.worldPosition[2] - player.aabb.half[2] + 0.5,
            },
            .half = {0.5, 0.5, 0.5}
        };

        if (player.ray.blockFound) {
            if (!sweepAABB(player.aabb, voxel, camera.motion).hit.result) {
                player_placeBlock();
                clicked = true;
            }
        }
    }

    clicked = !window.onMouseRelease;

    if (window.keyPressed[GLFW_KEY_1]) {
        player.selectedBlock = block_getID("dirt"); 
    } else if (window.keyPressed[GLFW_KEY_2]) {
        player.selectedBlock = block_getID("grass");
    } else if (window.keyPressed[GLFW_KEY_3]) {
        player.selectedBlock = block_getID("planks");
    } else if (window.keyPressed[GLFW_KEY_4]) {
        player.selectedBlock = block_getID("log");
    }
}

void player_raycast() {
    player.ray = ray_cast(camera.position, camera.front, player.reach);
}

void player_placeBlock() {
    struct Chunk *chunkToModify = player.ray.chunkToModify;

    if (player.ray.blockFound) {
        ivec3 blockPlaceLocation;

        switch (player.ray.placedDirection) {
            case FRONT:
                glm_ivec3_copy((ivec3){player.ray.blockFoundPosition[0], player.ray.blockFoundPosition[1], player.ray.blockFoundPosition[2] + 1}, blockPlaceLocation);
                break;
            case BACK:
                glm_ivec3_copy((ivec3){player.ray.blockFoundPosition[0], player.ray.blockFoundPosition[1], player.ray.blockFoundPosition[2] - 1}, blockPlaceLocation);
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
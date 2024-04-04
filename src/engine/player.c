#include "player.h"
#include "globals.h"
#include "block_overlay.h"

struct Player player; 

static bool player_collision(ivec3 position) {
    return (
        player.position[0] <= position[0] + 1 &&
        player.position[0] + player.dimensions[0] >= position[0] &&
        player.position[1] <= position[1] + 1 &&
        player.position[1] + player.dimensions[1] >= position[1] &&
        player.position[2] <= position[2] + 1 &&
        player.position[2] + player.dimensions[2] >= position[2]
    );
}

static float player_AABB(ivec3 position, float* normalX, float* normalZ) {
    float xInvEntry, zInvEntry; 
    float xInvExit, zInvExit;

    float xEntry, zEntry; 
    float xExit, zExit;

    if (camera.velocity[0] > 0.0f) { 
        xInvEntry = position[0] - (camera.position[0] + player.dimensions[0]); 
        xInvExit = (position[0] + 1) - camera.position[0];
    } else  { 
        xInvEntry = (position[0] + 1) - player.position[0];  
        xInvExit = position[0] - (player.position[0] + player.dimensions[0]);  
    } 

    if (camera.velocity[2] > 0.0f) { 
        zInvEntry = position[2] - (camera.position[2] + player.dimensions[2]); 
        zInvExit = (position[2] + 1) - camera.position[2];
    } else  { 
        zInvEntry = (position[2] + 1) - player.position[2];  
        zInvExit = position[2] - (player.position[2] + player.dimensions[2]);  
    }

    if (camera.velocity[0] == 0.0f) { 
        xEntry = -INFINITY;
        xExit = INFINITY;
    } else {
        xEntry = xInvEntry / camera.velocity[0]; 
        xExit = xInvExit / camera.velocity[0]; 
    } 

    if (camera.velocity[2] == 0.0f) { 
        zEntry = -INFINITY;
        zExit = INFINITY;
    } else {
        zEntry = zInvEntry / camera.velocity[2]; 
        zExit = zInvExit / camera.velocity[2]; 
    } 

    float entryTime = fmax(xEntry, zEntry); 
    float exitTime = fmin(xExit, zExit);

    if (entryTime > exitTime || xEntry < 0.0f && zEntry < 0.0f || xEntry > 1.0f || zEntry > 1.0f) { 
        *normalX = 0.0f; 
        *normalZ = 0.0f;
        return 1.0f; 
    } else {
        if (xEntry > zEntry) { 
            if (xInvEntry < 0.0f) { 
                *normalX = 1.0f; 
                *normalZ = 0.0f; 
            } else { 
                *normalX = -1.0f; 
                *normalZ = 0.0f;
            } 
        } else { 
            if (zInvEntry < 0.0f) { 
                *normalX = 0.0f; 
                *normalZ = 1.0f; 
            } else { 
                *normalX = 0.0f;
                *normalZ = -1.0f;
            } 
        }
        return entryTime; 
    }
}

void player_init() {
    player.FOV = globals.FOV;
    glm_vec3_copy((vec3){25.0f, 200.0f, 25.0f}, player.position);
    glm_vec3_copy((vec3){0.6, 1.8, 0.6}, player.dimensions);
    glm_ivec3_copy((ivec3){player.position[0] / CHUNK_SIZE, player.position[1] / CHUNK_SIZE, player.position[2] / CHUNK_SIZE}, player.chunkPosition);
    glm_ivec3_copy((ivec3){0, 0, 0}, player.previousPosition);

    camera_init(player.FOV, globals.mouseSensitivity, player.position);
    player.exitedChunk = false;

    player.breakTime = 0.0f;
    player.selectedBlock = block_getID("dirt");
    player.reach = globals.reach;
}

void player_update() {
    glm_vec3_copy(camera.position, player.position);
    glm_ivec3_copy((ivec3){player.position[0] / CHUNK_SIZE, player.position[1] / CHUNK_SIZE, player.position[2] / CHUNK_SIZE}, player.chunkPosition);
    for (int i = 0; i < 3; i++) {
        if (player.position[i] < 0) {
            player.chunkPosition[i] -= 1;
        }
    }

    struct Chunk* chunk = world_getChunk(player.chunkPosition);

    bool collision = false;
    ivec3 blockCollisionPosition;
    for (int x = 1; x < CS_P-1; x++) {
        for (int y = 1; y < CS_P-1; y++) {
            for (int z  = 1; z < CS_P-1; z++) { 
                if (chunk != NULL && chunk->voxels[blockIndex(x, y, z)] != BLOCK_AIR) {
                    glm_ivec3_copy((ivec3){
                        x + player.chunkPosition[0] * CHUNK_SIZE,
                        y + player.chunkPosition[1] * CHUNK_SIZE, 
                        z + player.chunkPosition[2] * CHUNK_SIZE
                    }, blockCollisionPosition);
                }
            }
        }
    }



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
    player.ray = ray_cast(camera.position, camera.front, player.reach);
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
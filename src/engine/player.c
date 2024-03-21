#include "player.h"

struct Player player; 

void player_init() {
    player.FOV = 60.0f;
    glm_vec3_copy((vec3){0.0f, 100.0f, 0.0f}, player.position);
    player.speed = (struct PlayerSpeed) {
        30.0f,
        60.0f,
        90.0f
    };

    glm_ivec3_copy((ivec3){player.position[0] / CHUNK_SIZE, player.position[1] / CHUNK_SIZE, player.position[2] / CHUNK_SIZE}, player.chunkPosition);
    glm_ivec3_copy((ivec3){0, 0, 0}, player.previousPosition);

    camera_init(player.FOV, player.speed.normal, 0.1f, player.position);
    player.exitedChunk = false;

    player.selectedBlock = block_getID("dirt");
    player.reach = 50.0f;
}

void player_update() {
    glm_vec3_copy(camera.position, player.position);
    glm_ivec3_copy((ivec3){player.position[0] / CHUNK_SIZE, player.position[1] / CHUNK_SIZE, player.position[2] / CHUNK_SIZE}, player.chunkPosition);

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

void player_placeBlock() {
    player.ray = ray_cast(camera.position, camera.front, player.reach);
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

void player_destroyBlock() {
    player.ray = ray_cast(camera.position, camera.front, player.reach);
    struct Chunk *chunkToModify = player.ray.chunkToModify;

    if (player.ray.blockFound) {
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

        //world_remeshChunk(chunkToModify->position);
        chunk_bind(world_getChunk(chunkToModify->position));
    }
}
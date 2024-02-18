#include "player.h"

struct Player player; 

void player_init() {
    player.FOV = 60.0f;
    player.speed = (struct PlayerSpeed) {
        30.0f,
        60.0f,
        90.0f
    };

    camera_init(player.FOV, player.speed.normal, 0.1f);

    player.selectedBlock = block_getID("dirt");
    player.reach = 50.0f;
}

void player_update() {
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
         player.selectedBlock = block_getID("snow");
    } else if (window.keyPressed[GLFW_KEY_8]) {
         player.selectedBlock = block_getID("lava");
    } else if (window.keyPressed[GLFW_KEY_9]) {
         player.selectedBlock = block_getID("leaves");
    } else if (window.keyPressed[GLFW_KEY_0]) {
         player.selectedBlock = block_getID("cobblestone");
    }
}

void player_placeBlock() {
    player.ray = ray_cast(camera.position, camera.front, player.reach);

    if (player.ray.blockFound) {
        ivec3 blockPlaceLocation;

        /*switch (player.ray.placedDirection) {
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
        }*/

        glm_ivec3_copy((ivec3){player.ray.blockFoundPosition[0], player.ray.blockFoundPosition[1] + 1, player.ray.blockFoundPosition[2]}, blockPlaceLocation);

        struct Chunk *chunkToModify = player.ray.chunkToModify;

        printf("Placed: [%d %d %d] (%d %d %d)\n", player.ray.chunkToModify->position[0], player.ray.chunkToModify->position[1], player.ray.chunkToModify->position[2], player.ray.blockFoundPosition[0], player.ray.blockFoundPosition[1], player.ray.blockFoundPosition[2]);

        chunkToModify->voxels[blockIndex(blockPlaceLocation[0], blockPlaceLocation[1], blockPlaceLocation[2])] = player.selectedBlock;
        chunkToModify->vertexList->size = 0;

        // This works
        if (blockPlaceLocation[0] == 1) {
            struct Chunk *c = world_getChunk((ivec3){chunkToModify->position[0] - 1, chunkToModify->position[1], chunkToModify->position[2]});
            c->vertexList->size = 0;
            world_meshChunk(c->position);
            puts("1");
        } else if (blockPlaceLocation[0] == CHUNK_SIZE) {
            struct Chunk *c = world_getChunk((ivec3){chunkToModify->position[0] + 1, chunkToModify->position[1], chunkToModify->position[2]});
            c->vertexList->size = 0;
            world_meshChunk(c->position);
            puts("2");
        }


        if (blockPlaceLocation[2] == 1) {
            struct Chunk *c = world_getChunk((ivec3){chunkToModify->position[0], chunkToModify->position[1], chunkToModify->position[2] - 1});
            c->vertexList->size = 0;
            world_meshChunk(c->position);
            puts("5");
        } else if (blockPlaceLocation[2] == CHUNK_SIZE) {
            struct Chunk *c = world_getChunk((ivec3){chunkToModify->position[0], chunkToModify->position[1], chunkToModify->position[2] + 1});
            c->vertexList->size = 0;
            world_meshChunk(c->position);
            puts("6");
        }

        /*if (blockPlaceLocation[2] == 1) {
            struct Chunk *c = world_getChunk((ivec3){chunkToModify->position[0], chunkToModify->position[1], chunkToModify->position[2] - 1});
            c->vertexList->size = 0;
            world_meshChunk(c->position);
            chunkToModify->voxels[blockIndex(player.ray.blockFoundPosition[0], player.ray.blockFoundPosition[1], 0)] = 4;
            puts("5");
        }*/

        world_meshChunk(chunkToModify->position);
    }
}

void player_destroyBlock() {
    player.ray = ray_cast(camera.position, camera.front, player.reach);
    struct Chunk *chunkToModify = player.ray.chunkToModify;

    if (player.ray.blockFound) {
        chunkToModify->vertexList->size = 0;
        chunkToModify->voxels[blockIndex(player.ray.blockFoundPosition[0], player.ray.blockFoundPosition[1], player.ray.blockFoundPosition[2])] = BLOCK_AIR;
        ivec3 newPosition;

        // Remesh neighboring chunk if the block destroyed lies on the edge of a chunk
        if (player.ray.blockFoundPosition[0] == 0) {
            glm_ivec3_copy((ivec3){chunkToModify->position[0] - 1, chunkToModify->position[1], chunkToModify->position[2]}, newPosition);

            world_getChunk(newPosition)->vertexList->size = 0;
            world_meshChunk(newPosition);
        }
        if (player.ray.blockFoundPosition[0] == CHUNK_SIZE - 1) {
            glm_ivec3_copy((ivec3){chunkToModify->position[0] + 1, chunkToModify->position[1], chunkToModify->position[2]}, newPosition);

            world_getChunk(newPosition)->vertexList->size = 0;
            world_meshChunk(newPosition);
        }

        if (player.ray.blockFoundPosition[1] == 0) {
            glm_ivec3_copy((ivec3){chunkToModify->position[0], chunkToModify->position[1] - 1, chunkToModify->position[2]}, newPosition);

            world_getChunk(newPosition)->vertexList->size = 0;
            world_meshChunk(newPosition);
        }
        if (player.ray.blockFoundPosition[1] == CHUNK_SIZE - 1) {
            glm_ivec3_copy((ivec3){chunkToModify->position[0], chunkToModify->position[1] + 1, chunkToModify->position[2]}, newPosition);

            world_getChunk(newPosition)->vertexList->size = 0;
            world_meshChunk(newPosition);
        }

        if (player.ray.blockFoundPosition[2] == 0) {
            glm_ivec3_copy((ivec3){chunkToModify->position[0], chunkToModify->position[1], chunkToModify->position[2] - 1}, newPosition);

            world_getChunk(newPosition)->vertexList->size = 0;
            world_meshChunk(newPosition);
        }
        if (player.ray.blockFoundPosition[2] == CHUNK_SIZE - 1) {
            glm_ivec3_copy((ivec3){chunkToModify->position[0], chunkToModify->position[1], chunkToModify->position[2] + 1}, newPosition);

            world_getChunk(newPosition)->vertexList->size = 0;
            world_meshChunk(newPosition);
        }

        world_meshChunk(chunkToModify->position);
    }
}
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

        // When placing on block faces, you have to check if you're on a border of a chunk, or else blockPosition.[x, z] ± 1 will be out of range.
        // Block positions are in a range from 0-15, therefore -1 and 16 (CHUNK_SIZE[X, Z]) are out of range
        if (blockPlaceLocation[0] == -1) {
            player.ray.chunkToModify = world_getChunk(world_hashChunk((ivec2s){world_unhashChunk(player.ray.chunkToModify->id).x - 1, world_unhashChunk(player.ray.chunkToModify->id).y}));
            blockPlaceLocation[0] = 15;
        } else if (blockPlaceLocation[0] == CHUNK_SIZE) {
            player.ray.chunkToModify = world_getChunk(world_hashChunk((ivec2s){world_unhashChunk(player.ray.chunkToModify->id).x + 1, world_unhashChunk(player.ray.chunkToModify->id).y}));
            blockPlaceLocation[0] = 0;
        }

        if (blockPlaceLocation[2] == -1) {
            player.ray.chunkToModify = world_getChunk(world_hashChunk((ivec2s){world_unhashChunk(player.ray.chunkToModify->id).x, world_unhashChunk(player.ray.chunkToModify->id).y - 1}));
            blockPlaceLocation[2] = 15;
        } else if (blockPlaceLocation[2] == CHUNK_SIZE) {
            player.ray.chunkToModify = world_getChunk(world_hashChunk((ivec2s){world_unhashChunk(player.ray.chunkToModify->id).x, world_unhashChunk(player.ray.chunkToModify->id).y + 1}));
            blockPlaceLocation[2] = 0;
        }

        player.ray.chunkToModify->meshSize = 0;
        player.ray.chunkToModify->blocks[blockIndex(blockPlaceLocation[0], blockPlaceLocation[1], blockPlaceLocation[2])].id = player.selectedBlock;

        world_meshChunk(world_unhashChunk(player.ray.chunkToModify->id));
    }
}

void player_destroyBlock() {
    player.ray = ray_cast(camera.position, camera.front, player.reach);

    if (player.ray.blockFound) {
        player.ray.chunkToModify->meshSize = 0;
        player.ray.chunkToModify->blocks[blockIndex(player.ray.blockFoundPosition[0], player.ray.blockFoundPosition[1], player.ray.blockFoundPosition[2])].id = BLOCK_AIR;
        ivec2s currentChunkPosition = world_unhashChunk(player.ray.chunkToModify->id);
        ivec2s newPosition;

        // Remesh neighboring chunk if the block destroyed lies on the edge of a chunk
        if (player.ray.blockFoundPosition[0] == 0) {
            newPosition = (ivec2s){currentChunkPosition.x - 1, currentChunkPosition.y};

            world_getChunk(world_hashChunk(newPosition))->meshSize = 0;
            world_meshChunk(world_unhashChunk(world_hashChunk(newPosition)));
        }
        if (player.ray.blockFoundPosition[0] == 15) {
            newPosition = (ivec2s){currentChunkPosition.x + 1, currentChunkPosition.y};

            world_getChunk(world_hashChunk(newPosition))->meshSize = 0;
            world_meshChunk(world_unhashChunk(world_hashChunk(newPosition)));
        }
        if (player.ray.blockFoundPosition[2] == 0) {
            newPosition = (ivec2s){currentChunkPosition.x, currentChunkPosition.y - 1};

            world_getChunk(world_hashChunk(newPosition))->meshSize = 0;
            world_meshChunk(world_unhashChunk(world_hashChunk(newPosition)));
        } if (player.ray.blockFoundPosition[2] == 15) {
            newPosition = (ivec2s){currentChunkPosition.x, currentChunkPosition.y + 1};

            world_getChunk(world_hashChunk(newPosition))->meshSize = 0;
            world_meshChunk(world_unhashChunk(world_hashChunk(newPosition)));
        }

        world_meshChunk(world_unhashChunk(player.ray.chunkToModify->id));
    }
}
#include "player.h"
#include "../world/world.h"

struct Player player;

static ivec2 prevChunkPos = (ivec2){0, 0};

void initPlayer() {
    glm_vec3_copy(camera.position, player.position);

    player.chunkPos[0] = (int) player.position[0] / CHUNK_SIZE_X;
    player.chunkPos[1] = (int) player.position[1] / CHUNK_SIZE_Z;

    player.prevPos[0] = 0;
    player.prevPos[1] = 0;
    
    glm_ivec2_copy(prevChunkPos, player.chunkPos);
    player.movedBetweenChunks = false;
}

void updatePlayer() {
    glm_vec3_copy(camera.position, player.position);

    player.chunkPos[0] = (int) player.position[0] / CHUNK_SIZE_X;
    player.chunkPos[1] = (int) player.position[2] / CHUNK_SIZE_Z;

    if ((player.chunkPos[0] != prevChunkPos[0]) || (player.chunkPos[1] != prevChunkPos[1])) {
        player.movedBetweenChunks = true;

        glm_ivec2_copy(player.chunkPos, prevChunkPos);
    }
}
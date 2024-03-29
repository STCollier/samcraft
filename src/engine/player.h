#ifndef PLAYER_H
#define PLAYER_H

#include "window.h"
#include "raycast.h"
#include "camera.h"
#include "shader.h"
#include "../world/world.h"

struct PlayerSpeed {
    float slow;
    float normal;
    float sprint;
};

struct Player {
    vec3 position; // Position of player
    unsigned int FOV; // Field of View
    unsigned int reach; // Reach of the player (to break and place blocks)
    struct PlayerSpeed speed; // Different speeds of the player
    struct Ray ray; // Ray emitted by the player
    int selectedBlock; // Block that the player selected

    bool exitedChunk;
    ivec3 previousPosition, chunkPosition;
};

void player_init();
void player_update();
void player_raycast();
void player_placeBlock();
void player_destroyBlock();

extern struct Player player;

#endif
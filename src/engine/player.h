#ifndef PLAYER_H
#define PLAYER_H

#include "window.h"
#include "raycast.h"
#include "camera.h"
#include "shader.h"
#include "physics.h"
#include "../world/world.h"

struct PlayerSpeed {
    float slow;
    float normal;
    float sprint;
};

struct Player {
    vec3 dimensions;

    unsigned int FOV; // Field of View
    unsigned int reach; // Reach of the player (to break and place blocks)
    struct PlayerSpeed speed; // Different speeds of the player
    struct Ray ray; // Ray emitted by the player
    int selectedBlock; // Block that the player selected
    float breakTime; // How long the player has been breaking a block
    ivec3 previousRayLookAt;
    struct AABB aabb;

    bool exitedChunk;
    ivec3 previousPosition, chunkPosition;
};

void player_init();
void player_update();
void player_raycast();
void player_placeBlock();
void player_destroyBlock(shader_t blockOverlayShader);

extern struct Player player;

#endif
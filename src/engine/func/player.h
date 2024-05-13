#include "../util/lerp.h"
#include "../core/shader.h"
#include "../func/physics.h"
#include "../func/raycast.h"

#ifndef PLAYER_H
#define PLAYER_H

#include "../util/common.h"

struct PlayerSpeed {
    float slow;
    float normal;
    float sprint;
};

enum PlayerState {
    SNEAK,
    WALK,
    SPRINT
};

struct Player {
    enum PlayerState state;
    struct AABB aabb;
    struct Ray ray; // Ray emitted by the player
    lerp_t cameraFOVLerp;
    vec3 dimensions; // Hitbox size of player
    bool exitedChunk;
    unsigned int FOV; // Field of View
    unsigned int reach; // Reach of the player (to break and place blocks)
    int selectedBlock; // Block that the player selected
    float breakTime; // How long the player has been breaking a block
    ivec3 previousRayLookAt, previousPosition, chunkPosition;

    int currentFOV;
};

void player_init();
void player_update(shader_t blockOverlayShader);
void player_raycast();
void player_placeBlock();
void player_destroyBlock(shader_t blockOverlayShader);

extern struct Player player;

#endif
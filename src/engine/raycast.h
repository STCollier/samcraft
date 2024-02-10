#ifndef RAYCAST_H
#define RAYCAST_H

#include "cglm/cglm.h"
#include "cglm/call.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../world/chunk.h"
#include "../world/block.h"
#include "../world/world.h"

struct Ray {
    bool blockFound;
    struct Chunk *chunkToModify;
    vec3 blockFoundPosition;
    int maxDistance;
};

struct Ray ray_cast(vec3 startPosition, vec3 rayDirection);

#endif
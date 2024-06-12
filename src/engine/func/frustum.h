#ifndef FRUSTUM_H
#define FRUSTUM_H

#include "../util/common.h"
#include "../../world/chunk.h"

struct Frustum {
    vec4 planes[6];
    vec4 corners[8];
};

struct Frustum updateCameraFrustum();
bool boxInFrustum(struct Frustum frustum, struct Chunk chunk);

#endif
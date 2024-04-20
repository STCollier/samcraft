#ifndef PHYSICS_H
#define PHYSICS_H

#include "cglm/cglm.h"
#include "cglm/call.h"

#include "util.h"

struct AABB {
    vec3 position;
    vec3 half;
};

struct Hit {
    struct AABB collider;
    bool result;
    vec3 position, delta;
    ivec3 normal;
    float time;
};

struct Sweep {
    struct Hit hit;
    vec3 position;
    float time;
};

struct Hit intersectAABB(struct AABB aabb, struct AABB box);
struct Sweep sweepAABB(struct AABB aabb, struct AABB box, vec3 delta);
struct Sweep sweepInto(struct AABB aabb, struct AABB* colliders, size_t numColliders, vec3 delta);

#endif
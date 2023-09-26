#ifndef RAY_H
#define RAY_H

#include "cglm/cglm.h"
#include "cglm/call.h"

struct Ray {
    vec3 origin;
    vec3 dir;
    vec3 invDir;
    float length;
};

struct Box {
    vec3 min;
    vec3 max;
};

struct Ray ray(vec3 origin, vec3 dir, float length);
void updateRay(struct Ray *ray, vec3 origin, vec3 direction);
bool rayIntersection(const struct Ray ray, const struct Box box);

#endif
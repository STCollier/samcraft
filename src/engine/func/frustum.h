#ifndef FRUSTUM_H
#define FRUSTUM_H

#include "../util/common.h"

struct Plane {
    vec3 normal; // Unit vector
    vec3 distance; // Distance from the origin to the nearest point in the plane
};

struct Frustum {
    struct Plane top;
    struct Plane bottom;

    struct Plane right;
    struct Plane left;

    struct Plane far;
    struct Plane near;
};

#endif
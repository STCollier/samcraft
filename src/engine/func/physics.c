#include "physics.h"

float max2(float x, float y) {
   return (x > y) ? x : y;
}

float max3(float m, float n, float p) {
   return max2(max2(m, n), p);
}

float min2(float x, float y) {
   return (x < y) ? x : y;
}

float min3(float m, float n, float p) {
   return min2(min2(m, n), p);
}

struct Hit intersectSegment(struct AABB aabb, vec3 position, vec3 delta, vec3 padding) {
    struct Hit hit;

    hit.result = false;

    vec3 scale_ = {1.0 / delta[0], 1.0 / delta[1], 1.0 / delta[2]};
    ivec3 sign_ = {sign(scale_[0]), sign(scale_[1]), sign(scale_[2])};

    vec3 nearTime = {
        (aabb.position[0] - sign_[0] * (aabb.half[0] + padding[0]) - position[0]) * scale_[0],
        (aabb.position[1] - sign_[1] * (aabb.half[1] + padding[1]) - position[1]) * scale_[1],
        (aabb.position[2] - sign_[2] * (aabb.half[2] + padding[2]) - position[2]) * scale_[2],
    };
    vec3 farTime = {
        (aabb.position[0] + sign_[0] * (aabb.half[0] + padding[0]) - position[0]) * scale_[0],
        (aabb.position[1] + sign_[1] * (aabb.half[1] + padding[1]) - position[1]) * scale_[1],
        (aabb.position[2] + sign_[2] * (aabb.half[2] + padding[2]) - position[2]) * scale_[2],
    };

    if (nearTime[0] > farTime[1] || 
        nearTime[0] > farTime[2] || 
        nearTime[1] > farTime[0] ||
        nearTime[1] > farTime[2] ||
        nearTime[2] > farTime[0] ||
        nearTime[2] > farTime[1] ) {
        hit.result = false;
        return hit;
    }


    const float nearTime_c = max3(nearTime[0], nearTime[1], nearTime[2]);
    const float farTime_c = min3(farTime[0], farTime[1], farTime[2]);

    if (nearTime_c >= 1 || farTime_c <= 0) {
        hit.result = false;
        return hit;
    }

    hit.time = clamp(nearTime_c, 0, 1);
    hit.collider = aabb;
    hit.result = true;

    if (nearTime[0] > nearTime[1] && nearTime[0] > nearTime[2]) {
        hit.normal[0] = -sign_[0];
        hit.normal[1] = 0;
        hit.normal[2] = 0;
    } else if (nearTime[1] > nearTime[0] && nearTime[1] > nearTime[2]) {
        hit.normal[0] = 0;
        hit.normal[1] = -sign_[1];
        hit.normal[2] = 0;
    } else {
        hit.normal[0] = 0;
        hit.normal[1] = 0;
        hit.normal[2] = -sign_[2];
    }

    glm_vec3_copy((vec3){
        (1.0 - hit.time) * -delta[0],
        (1.0 - hit.time) * -delta[1],
        (1.0 - hit.time) * -delta[2],
    }, hit.delta);

    glm_vec3_copy((vec3){
        position[0] + delta[0] * hit.time,
        position[1] + delta[1] * hit.time,
        position[2] + delta[2] * hit.time
    }, hit.position);

    return hit;
}

struct Hit intersectAABB(struct AABB aabb, struct AABB box) {
    struct Hit hit;

    glm_vec3_copy((vec3){0, 0, 0}, hit.delta);
    glm_ivec3_copy((ivec3){0, 0, 0}, hit.normal);

    const float dx = box.position[0] - aabb.position[0];
    const float px = (box.half[0] + aabb.half[0]) - fabsf(dx);
    if (px <= 0) {
        hit.result = false;
        return hit;
    }

    const float dy = box.position[1] - aabb.position[1];
    const float py = (box.half[1] + aabb.half[1]) - fabsf(dy);
    if (py <= 0) {
        hit.result = false;
        return hit;
    }

    const float dz = box.position[2] - aabb.position[2];
    const float pz = (box.half[2] + aabb.half[2]) - fabsf(dz);
    if (pz <= 0) {
        hit.result = false;
        return hit;
    }

    hit.result = true;
    hit.collider = aabb;

    if (px < py && px < pz) {
        const int sx = sign(dx);
        hit.delta[0] = px * sx;
        hit.normal[0] = sx;
        hit.position[0] = aabb.position[0] + (aabb.half[0] * sx);
        hit.position[1] = box.position[1];
        hit.position[2] = box.position[2];
    } else if (py < px && py < pz) {
        const int sy = sign(dy);
        hit.delta[1] = py * sy;
        hit.normal[1] = sy;
        hit.position[0] = box.position[0];
        hit.position[1] = aabb.position[1] + (aabb.half[1] * sy);
        hit.position[2] = box.position[2];
    } else {
        const int sz = sign(dz);
        hit.delta[2] = pz * sz;
        hit.normal[2] = sz;
        hit.position[0] = box.position[0];
        hit.position[1] = box.position[1];
        hit.position[2] = aabb.position[2] + (aabb.half[2] * sz);
    }

    return hit;
}

struct Sweep sweepAABB(struct AABB aabb, struct AABB box, vec3 delta) {
    struct Sweep sweep;

    if (delta[0] == 0 && delta[1] == 0 && delta[2] == 0) {
        glm_vec3_copy(box.position, sweep.position);
        sweep.hit = intersectAABB(aabb, box);
        sweep.time = sweep.hit.result ? (sweep.hit.time = 0) : 1;
        return sweep;
    }

    sweep.hit = intersectSegment(box, aabb.position, delta, aabb.half);

    if (sweep.hit.result) {
        sweep.time = clamp(sweep.hit.time - EPSILON, 0.0, 1.0);
        sweep.position[0] = box.position[0] + delta[0] * sweep.time;
        sweep.position[1] = box.position[1] + delta[1] * sweep.time;
        sweep.position[2] = box.position[2] + delta[2] * sweep.time;

        vec3 direction = {delta[0], delta[1], delta[2]};
        float length = direction[0] * direction[1] + direction[1] * direction[0] + direction[2] * direction[2];
          if (length > 0) {
            length = sqrt(length);
            const float inverseLength = 1.0 / length;
            direction[0] *= inverseLength;
            direction[1] *= inverseLength;
            direction[2] *= inverseLength;
          } else {
            direction[0] = 1;
            direction[1] = 1;
            direction[2] = 1;
          }
        sweep.hit.position[0] = clamp(
            sweep.hit.position[0] + direction[0] * box.half[0],
            aabb.position[0] - aabb.half[0], aabb.position[0] + aabb.half[0]
        );
        sweep.hit.position[1] = clamp(
            sweep.hit.position[1] + direction[1] * box.half[1],
            aabb.position[1] - aabb.half[1], aabb.position[1] + aabb.half[1]
        );
        sweep.hit.position[2] = clamp(
            sweep.hit.position[2] + direction[2] * box.half[2],
            aabb.position[2] - aabb.half[2], aabb.position[2] + aabb.half[2]
        );
    } else {
        sweep.position[0] = box.position[0] + delta[0];
        sweep.position[1] = box.position[1] + delta[1];
        sweep.position[2] = box.position[2] + delta[2];
        sweep.time = 1;
    }

    return sweep;
}


struct Sweep sweepInto(struct AABB aabb, struct AABB* colliders, size_t numColliders, vec3 delta) {
    struct Sweep nearest;

    nearest.hit.result = false;

    nearest.time = 1;
    nearest.position[0] = aabb.position[0] + delta[0];
    nearest.position[1] = aabb.position[1] + delta[1];
    nearest.position[2] = aabb.position[2] + delta[2];

    for (int i = 0; i < numColliders; i++) {
        struct Sweep sweep = sweepAABB(aabb, colliders[i], delta);
        if (sweep.time < nearest.time) {
            nearest = sweep;
        }
    }

    return nearest;
}
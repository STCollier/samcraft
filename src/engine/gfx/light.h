#ifndef LIGHT_H
#define LIGHT_H

#include "../util/common.h"

struct DepthMap {
    unsigned int FBO, map, resolution;
};

struct Light {
    struct DepthMap depthMap;
    mat4 spaceMatrix;
    vec3 sunPosition;
};

void light_init();
void light_beginPass(shader_t shader);

extern struct Light light;

#endif
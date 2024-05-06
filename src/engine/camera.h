#ifndef CAMERA_H
#define CAMERA_H

#include "cglm/cglm.h"
#include "cglm/call.h"

#include "shader.h"

struct Camera {
    mat4 projection, view, model;
    vec3 position, tempPosition, oldPosition, delta;
    vec3 velocity, acceleration, motion;
    vec3 front, up, right;
    float speedValue[3], speed;
    float near, far, sensitivity, fov;
};

void camera_init(float fov, float sensitivity, vec3 position);
void camera_use(shader_t shader);
void camera_mouseCallback(double xposIn, double yposIn);

extern struct Camera camera;

#endif
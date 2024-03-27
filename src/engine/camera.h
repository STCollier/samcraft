#ifndef CAMERA_H
#define CAMERA_H

#include "cglm/cglm.h"
#include "cglm/call.h"

#include "shader.h"

struct Camera {
    mat4 projection;
    mat4 view;
    mat4 model;

    vec3 position;
    vec3 front;
    vec3 up;
    vec3 right;
    vec3 worldUp;

    float speedValue[3];
    float speed;

    float near, far, sensitivity, fov;
};

void camera_init(float fov, float sensitivity, vec3 position);
void camera_use(shader_t shader);
void camera_mouseCallback(double xposIn, double yposIn);

extern struct Camera camera;

#endif
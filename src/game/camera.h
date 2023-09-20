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

    float sensitivity;
    float speed;
    float fov;
};

void initCamera(float fov, float speed, float sensitivity);
void useCamera(struct Shader shader);
void cameraMouseCallback();

extern struct Camera camera;

#endif
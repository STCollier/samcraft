#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "window.h"
#include "shader.h"
#include "globals.h"
#include "camera.h"

struct Camera camera; // Define globally

float yaw = 45.0f;
float pitch = 0.0f;
float lastX = 0;
float lastY = 0;

void camera_init(float fov, float sensitivity, vec3 position) {
    camera.fov = fov;
    camera.sensitivity = sensitivity;

    camera.near = 0.01f;
    camera.far = 10000.0f;

    camera.speedValue[0] = globals.playerSpeed.slow;
    camera.speedValue[1] = globals.playerSpeed.normal;
    camera.speedValue[2] = globals.playerSpeed.sprint;

    camera.speed = camera.speedValue[1];

    glm_vec3_copy(position, camera.position);
    glm_vec3_copy(GLM_VEC3_ZERO, camera.velocity);
    glm_vec3_copy(GLM_VEC3_ZERO, camera.acceleration);

    glm_vec3_copy(GLM_VEC3_ZERO, camera.front);
    glm_vec3_copy((vec3){0.0f, 1.0f, 0.0f}, camera.up);

    lastX = window.width / 2.0;
    lastY = window.height / 2.0;

    LOG("Camera successfully loaded!");
}

void camera_use(shader_t shader) {
    shader_use(shader);

    glm_mat4_identity(camera.projection);
    glm_perspective(glm_rad(camera.fov), (float) window.width / (float) window.height, camera.near, camera.far, camera.projection); // Make sure to convert to floats for float division
    shader_setMat4(shader, "projection", camera.projection);

    glm_mat4_identity(camera.view);
    vec3 result;
    glm_vec3_add(camera.position, camera.front, result);
    glm_lookat(camera.position, result, camera.up, camera.view);
    shader_setMat4(shader, "view", camera.view);
}

void camera_mouseCallback(double xposIn, double yposIn) {
    float xpos = xposIn;
    float ypos = yposIn;

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // Reversed since y-coordinates go from bottom to top
    lastX = xpos;
    lastY = ypos;

    xoffset *= camera.sensitivity;
    yoffset *= camera.sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    // Make sure that when pitch is out of bounds, screen doesn't get flipped
    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    vec3 front;
    front[0] = cos(glm_rad(yaw)) * cos(glm_rad(pitch));
    front[1] = sin(glm_rad(pitch));
    front[2] = sin(glm_rad(yaw)) * cos(glm_rad(pitch));
    glm_vec3_normalize(front);
    glm_vec3_copy(front, camera.front);

    vec3 right;
    glm_vec3_cross(camera.front, (vec3){0.0, 1.0, 0.0}, right);
    glm_vec3_normalize(right);
    glm_vec3_copy(right, camera.right);

    vec3 up;
    glm_vec3_cross(camera.right, camera.front, up);
    glm_vec3_normalize(up);
    glm_vec3_copy(up, camera.up);
}
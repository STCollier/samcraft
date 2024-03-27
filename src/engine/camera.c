#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <math.h>

#include "window.h"
#include "shader.h"
#include "camera.h"
#include "util.h"
#include "globals.h"

//Define globally
struct Camera camera;

float yaw = 45.0f;
float pitch = 0.0f;
float lastX = 0;
float lastY = 0;

void camera_init(float fov, float sensitivity, vec3 position) {
    camera.fov = fov;
    camera.sensitivity = sensitivity;

    camera.near = 0.1f;
    camera.far = 10000.0f;

    camera.speedValue[0] = globals.playerSpeed.slow;
    camera.speedValue[1] = globals.playerSpeed.normal;
    camera.speedValue[2] = globals.playerSpeed.sprint;

    camera.speed = camera.speedValue[1];

    glm_vec3_copy(position, camera.position);
    glm_vec3_copy((vec3){0.0f, 0.0f, 0.0f}, camera.front);
    glm_vec3_copy((vec3){0.0f, 1.0f, 0.0f}, camera.up);
    glm_vec3_copy((vec3){0.0f, 1.0f, 0.0f}, camera.worldUp);

    lastX = window.width / 2.0;
    lastY = window.height / 2.0;

    LOG("Camera successfully loaded!");
}

static void camera_handleKeyboard() {
    float cameraSpeed = camera.speed * window.dt;

    if (glfwGetKey(window.self, GLFW_KEY_W) == GLFW_PRESS) { // forward
        vec3 mult;
        glm_vec3_scale(camera.front, cameraSpeed, mult);
        glm_vec3_add(camera.position, mult, camera.position);
    }

    if (glfwGetKey(window.self, GLFW_KEY_S) == GLFW_PRESS) { // backward
        vec3 mult;
        glm_vec3_scale(camera.front, cameraSpeed, mult);
        glm_vec3_sub(camera.position, mult, camera.position);
    }

    if (glfwGetKey(window.self, GLFW_KEY_A) == GLFW_PRESS) { // left
        vec3 mult;
        glm_vec3_scale(camera.right, cameraSpeed, mult);
        glm_vec3_sub(camera.position, mult, camera.position);
    }

    if (glfwGetKey(window.self, GLFW_KEY_D) == GLFW_PRESS) { // right
        vec3 mult;
        glm_vec3_scale(camera.right, cameraSpeed, mult);
        glm_vec3_add(camera.position, mult, camera.position);
    } 

    if (glfwGetKey(window.self, GLFW_KEY_E) == GLFW_PRESS) {
        camera.speed = camera.speedValue[2];
    } else if (glfwGetKey(window.self, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        camera.speed = camera.speedValue[0];
    } else {
        camera.speed = camera.speedValue[1];
    }
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

    camera_handleKeyboard();
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
    glm_vec3_cross(camera.front, camera.worldUp, right);
    glm_vec3_normalize(right);
    glm_vec3_copy(right, camera.right);

    vec3 up;
    glm_vec3_cross(camera.right, camera.front, up);
    glm_vec3_normalize(up);
    glm_vec3_copy(up, camera.up);
}

/*float x = (2.0f * window.mouseX) / window.width - 1.0f;
float y = 1.0f - (2.0f * window.mouseY) / window.height;
float z = 1.0f;
vec3 rayNDS = {x, y, z}; // Normalized device coordinate space
vec4 rayClip = {rayNDS[0], rayNDS[1], -1.0, 1.0};

mat4 inverseProjection;
glm_mat4_inv(camera.projection, inverseProjection);

vec4 rayEye;
glm_mat4_mulv(inverseProjection, rayClip, rayEye);
glm_vec4_copy((vec4){rayEye[0], rayEye[1], -1.0, 0.0}, rayEye);

mat4 inverseView;
glm_mat4_inv(camera.view, inverseView);

vec4 rayW;
glm_mat4_mulv(inverseView, rayEye, rayW);
vec3 rayWorld;
glm_vec3_normalize_to((vec3){rayW[0], rayW[1], rayW[2]}, rayWorld);

printf("%f %f %f\n", rayWorld[0], rayWorld[1], rayWorld[2]);*/
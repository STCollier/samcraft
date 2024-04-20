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

float directionV;
vec2 directionH;

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
    glm_vec3_copy((vec3){0.0f, 0.0f, 0.0f}, camera.velocity);
    glm_vec3_copy((vec3){0.0f, 0.0f, 0.0f}, camera.acceleration);

    glm_vec3_copy((vec3){0.0f, 0.0f, 0.0f}, camera.front);
    glm_vec3_copy((vec3){0.0f, 1.0f, 0.0f}, camera.up);
    glm_vec3_copy((vec3){0.0f, 1.0f, 0.0f}, camera.worldUp);

    lastX = window.width / 2.0;
    lastY = window.height / 2.0;

    LOG("Camera successfully loaded!");
}

static void camera_handleKeyboard() {
    if (glfwGetKey(window.self, GLFW_KEY_S) == GLFW_PRESS && camera.velocity[2] > -camera.speed) {
        camera.acceleration[2] = -0.1;
    } else if (glfwGetKey(window.self, GLFW_KEY_W) == GLFW_PRESS && camera.velocity[2] < camera.speed) {
        camera.acceleration[2] = 0.1;
    } else if (fabsf(camera.velocity[2]) > 0.2) {
        camera.acceleration[2] = (camera.velocity[2] < 0) ? 0.15 : -0.15;
    } else {
        camera.velocity[2] = 0;
        camera.acceleration[2] = 0;
    }

    vec3 m1;
    camera.velocity[2] += camera.acceleration[2];
    glm_vec3_scale((vec3){directionH[0], 0, directionH[1]}, camera.velocity[2] * window.dt, m1);

    /*if (glfwGetKey(window.self, GLFW_KEY_SPACE) == GLFW_PRESS && camera.velocity[1] == 0 && camera.acceleration[1] == 0) {
        camera.velocity[1] = -7;
    }
    camera.acceleration[1] -= 0.7; // Gravity
    camera.velocity[1] += camera.acceleration[1];
    camera.position[1] += camera.velocity[1] * window.dt;
    camera.acceleration[1] = 0; // Reset acceleration*/


   if (glfwGetKey(window.self, GLFW_KEY_A) == GLFW_PRESS && camera.velocity[0] < camera.speed) {
        camera.acceleration[0] = -0.1;
    } else if (glfwGetKey(window.self, GLFW_KEY_D) == GLFW_PRESS && camera.velocity[0] > -camera.speed) {
        camera.acceleration[0] = 0.1;
    } else if (fabsf(camera.velocity[0]) > 0.2) {
        camera.acceleration[0] = (camera.velocity[0] < 0) ? 0.15 : -0.15;
    } else {
        camera.velocity[0] = 0;
        camera.acceleration[0] = 0;
    }

    vec3 m2;
    camera.velocity[0] += camera.acceleration[0];
    glm_vec3_scale(camera.right, camera.velocity[0] * window.dt, m2);

    glm_vec3_add(m1, m2, camera.motion);


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
    glm_vec3_add(camera.position, camera.direction, result);
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

    camera.angle[0] += xoffset * camera.sensitivity;
    camera.angle[1] += yoffset * camera.sensitivity;

    if (camera.angle[1] < 0.01) {
        camera.angle[1] = 0.01;
    }
    if (TAU / 2.0 < camera.angle[1] - 0.01) {
        camera.angle[1] = TAU / 2.0 - 0.01;
    }

    directionV = -cosf(camera.angle[1]);
    glm_vec2_copy((vec2){
        cosf(camera.angle[0]),
        sinf(camera.angle[0])
    }, directionH);

    glm_vec3_scale(directionH, sqrtf(1.0 - directionV*directionV), directionH);
    glm_vec3_scale(directionH, camera.angle[1] <= 0 ? -1.0 : 1.0, directionH);

    glm_vec3_copy((vec3){
        directionH[0],
        clamp(directionV, -0.99, 0.99),
        directionH[1]
    }, camera.direction);

   //printf("%f %f %f\n", camera.direction[0], camera.direction[1], camera.direction[2]);

    /*xoffset *= camera.sensitivity;
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
    glm_vec3_copy(up, camera.up);*/
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
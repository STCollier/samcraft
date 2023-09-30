#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <math.h>

#include "window.h"
#include "shader.h"
#include "camera.h"
#include "util.h"

//Define globally
struct Camera camera;

float yaw = 45.0f;
float pitch = 0.0f;
float lastX = 0;
float lastY = 0;

void initCamera(float fov, float speed, float sensitivity) {
    camera.fov = fov;
    camera.speed = speed;
    camera.sensitivity = sensitivity;

    glm_vec3_copy((vec3){0.0f, 140.0f, 0.0f}, camera.position);
    glm_vec3_copy((vec3){0.0f, 0.0f, -1.0f}, camera.front);
    glm_vec3_copy((vec3){0.0f, 1.0f, 0.0f}, camera.up);

    lastX = window.width / 2.0;
    lastY = window.height / 2.0;

    LOG("Camera successfully loaded!");
}

static void handleCameraKeyboard() {
    float cameraSpeed = camera.speed * window.dt;

    if (glfwGetKey(window.self, GLFW_KEY_W) == GLFW_PRESS) {
        vec3 forwardDir;
        glm_vec3_scale(camera.front, cameraSpeed, forwardDir);
        glm_vec3_add(camera.position, forwardDir, camera.position);
    }

    if (glfwGetKey(window.self, GLFW_KEY_S) == GLFW_PRESS) {
        vec3 backwardsDir;
        glm_vec3_scale(camera.front, cameraSpeed, backwardsDir);
        glm_vec3_sub(camera.position, backwardsDir, camera.position);
    }

    if (glfwGetKey(window.self, GLFW_KEY_A) == GLFW_PRESS) {
        vec3 leftDir;
        glm_cross(camera.front, camera.up, leftDir);
        glm_normalize(leftDir);

        vec3 offset;
        glm_vec3_scale(leftDir, cameraSpeed, offset);
        glm_vec3_sub(camera.position, offset, camera.position);
    }

    if (glfwGetKey(window.self, GLFW_KEY_D) == GLFW_PRESS) {
        vec3 rightDir;
        glm_cross(camera.front, camera.up, rightDir);
        glm_normalize(rightDir);

        vec3 offset;
        glm_vec3_scale(rightDir, cameraSpeed, offset);

        glm_vec3_add(camera.position, offset, camera.position);
    } 

    if (glfwGetKey(window.self, GLFW_KEY_R) == GLFW_PRESS) {
        camera.speed = 50.0f;
    } else {
        camera.speed = 25.0f;
    }
}

void useCamera(struct Shader shader) {
    useShader(shader);

    glm_mat4_identity(camera.projection);
    glm_perspective(glm_rad(camera.fov), (float) window.width / (float) window.height, 0.1f, 2048.0f, camera.projection); // Make sure to convert to floats for float division
    setShaderMat4(shader, "projection", camera.projection);

    glm_mat4_identity(camera.view);
    vec3 result;
    glm_vec3_add(camera.position, camera.front, result);
    glm_lookat(camera.position, result, camera.up, camera.view);
    setShaderMat4(shader, "view", camera.view);

    handleCameraKeyboard();
}


void cameraMouseCallback(double xposIn, double yposIn) {
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
}
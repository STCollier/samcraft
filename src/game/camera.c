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

vec3 cameraPos = {0.0f, 3.0f, 0.0f};
vec3 cameraFront = {0.0f, 0.0f, -1.0f};
vec3 cameraUp = {0.0f, 1.0f, 0.0f};

float yaw = -45.0f;
float pitch = 0.0f;
float lastX = 0;
float lastY = 0;

void initCamera(float fov, float speed, float sensitivity) {
    camera.fov = fov;
    camera.speed = speed;
    camera.sensitivity = sensitivity;

    lastX = window.width / 2.0;
    lastY = window.height / 2.0;

    LOG("Camera successfully loaded!");
}

static void handleCameraKeyboard() {
    float cameraSpeed = camera.speed * window.dt;

    if (glfwGetKey(window.self, GLFW_KEY_W) == GLFW_PRESS) {
        vec3 forwardDir;
        glm_vec3_scale(cameraFront, cameraSpeed, forwardDir);
        glm_vec3_add(cameraPos, forwardDir, cameraPos);
    }

    if (glfwGetKey(window.self, GLFW_KEY_S) == GLFW_PRESS) {
        vec3 backwardsDir;
        glm_vec3_scale(cameraFront, cameraSpeed, backwardsDir);
        glm_vec3_sub(cameraPos, backwardsDir, cameraPos);
    }

    if (glfwGetKey(window.self, GLFW_KEY_A) == GLFW_PRESS) {
        vec3 leftDir;
        glm_cross(cameraFront, cameraUp, leftDir);
        glm_normalize(leftDir);

        vec3 offset;
        glm_vec3_scale(leftDir, cameraSpeed, offset);
        glm_vec3_sub(cameraPos, offset, cameraPos);
    }

    if (glfwGetKey(window.self, GLFW_KEY_D) == GLFW_PRESS) {
        vec3 rightDir;
        glm_cross(cameraFront, cameraUp, rightDir);
        glm_normalize(rightDir);

        vec3 offset;
        glm_vec3_scale(rightDir, cameraSpeed, offset);

        glm_vec3_add(cameraPos, offset, cameraPos);
    } 
}

void useCamera(struct Shader shader) {
    useShader(shader);

    glm_mat4_identity(camera.projection);
    glm_perspective(glm_rad(camera.fov), (float) window.width / (float) window.height, 0.1f, 500.0f, camera.projection); // Make sure to convert to floats for float division
    setShaderMat4(shader, "projection", camera.projection);

    glm_mat4_identity(camera.view);
    vec3 result;
    glm_vec3_add(cameraPos, cameraFront, result);
    glm_lookat(cameraPos, result, cameraUp, camera.view);
    setShaderMat4(shader, "view", camera.view);

    handleCameraKeyboard();
}

void cameraMouseCallback(double xposIn, double yposIn) {
    float xpos = xposIn;
    float ypos = yposIn;

    /*printf("%f %f\n", window.width/2 - xpos, window.width/2 - ypos);
    glfwSetCursorPos(window.self, window.width/2, window.height/2);*/

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

    glm_vec3_copy(front, cameraFront);
}
#include "../core/camera.h"

#include "light.h"
#include "../core/globals.h"

struct Light light;

void light_init() {
    // Init depthmap
    glGenFramebuffers(1, &light.depthMap.FBO);

    light.depthMap.resolution = 16384;

    glGenTextures(1, &light.depthMap.map);
    glBindTexture(GL_TEXTURE_2D, light.depthMap.map);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, light.depthMap.resolution, light.depthMap.resolution, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, light.depthMap.FBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, light.depthMap.map, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void light_beginPass(shader_t shader) {
    mat4 lightProjection, lightView;

    glm_ortho(-250.0f, 250.0f, -250.0f, 250.0f, camera.near, camera.far, lightProjection);
    glm_lookat(
        (vec3){200.0f, 200.0f, 200.0f},
        (vec3){0.0f, 0.0f, 0.0},
        (vec3){0.0f, 1.0f, 0.0f},
    lightView);
    glm_mat4_mul(lightProjection, lightView, light.spaceMatrix);    

    shader_use(shader);
    shader_setMat4(shader, "lightSpaceMatrix", light.spaceMatrix);

    glViewport(0, 0, light.depthMap.resolution, light.depthMap.resolution);
    glBindFramebuffer(GL_FRAMEBUFFER, light.depthMap.FBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    // renderScene()
}
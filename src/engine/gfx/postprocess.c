#include "postprocess.h"

static unsigned int hdrFBO, colorBuffer, hdrQuadVBO;

static float quadVertices[] = {
    -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
    -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
    1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
    1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
};

void HDR_init() {
    glGenFramebuffers(1, &hdrFBO);
    // create floating point color buffer
    glGenTextures(1, &colorBuffer);
    glBindTexture(GL_TEXTURE_2D, colorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, window.width, window.height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // create depth buffer (renderbuffer)
    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, window.width, window.height);
    // attach buffers
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBuffer, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        ERROR("HDR Framebuffer not complete!")
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void HDR_set() {
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

static float exposure = 1.0;
static bool useHdr = true;
static bool clicked = false;

void HDR_use(shader_t shader) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
    shader_use(shader);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, colorBuffer);
    shader_setInt(shader, "hdr", useHdr);
    shader_setInt(shader, "hdrBuffer", 0);

    if (glfwGetKey(window.self, GLFW_KEY_UP) == GLFW_PRESS) {
        exposure += 0.01;
    }
    if (glfwGetKey(window.self, GLFW_KEY_DOWN) == GLFW_PRESS) {
        exposure -= 0.01;
    }
    if (glfwGetKey(window.self, GLFW_KEY_BACKSLASH) == GLFW_PRESS && !clicked) {
        useHdr = !useHdr;
    }

    shader_setFloat(shader, "exposure", exposure);

    glGenVertexArrays(1, &hdrQuadVBO);
    glGenBuffers(1, &hdrQuadVBO);
    glBindVertexArray(hdrQuadVBO);
    glBindBuffer(GL_ARRAY_BUFFER, hdrQuadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    glBindVertexArray(hdrQuadVBO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}
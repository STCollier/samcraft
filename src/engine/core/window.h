#ifndef WINDOW_H
#define WINDOW_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "../util/common.h"
#include "../util/timer.h"

struct Window {
    GLFWwindow* self;
    const char* title;
    int width;
    int height;

    float dt; // Delta Time
    float lastFrame;
    float aspectRatio;
    bool leftClicked;
    bool rightClicked;
    bool onMouseRelease;

    struct Timer FPSTimer;
    int FPS;

    int keyPressed[1028];

    double mouseX;
    double mouseY;
};

extern struct Window window;

void window_create(const char* title, int width, int height);
void window_destroy();
void window_update();

#endif
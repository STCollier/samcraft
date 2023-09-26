#ifndef WINDOW_H
#define WINDOW_H

#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <stdbool.h>

struct Window {
    GLFWwindow* self;
    const char* title;
    int width;
    int height;

    float dt; // Delta Time
    float lastFrame;
    bool leftClicked;
    bool rightClicked;
};

extern struct Window window;

void createWindow(const char* title, int width, int height);
void terminateWindow();
void updateWindow();

#endif
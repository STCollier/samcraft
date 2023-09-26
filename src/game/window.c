#include "window.h"
#include "camera.h"
#include "util.h"

#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>


struct Window window;

static void _sizeCallback() {
    glViewport(0, 0, window.width, window.height);
}

static void _keyboardCallback() {
    if (glfwGetKey(window.self, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window.self, 1);
}

static void _mouseCallback(GLFWwindow* _window, double xpos, double ypos) {
    (void) _window;
    cameraMouseCallback(xpos, ypos);
}

static void _mouseButtonCallback(GLFWwindow* _window, int button, int action, int mods) {
    (void) _window;
    (void) mods;
    
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
       window.leftClicked = true;
    } else {
        window.leftClicked = false;
    }

    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
       window.rightClicked = true;
    } else {
        window.rightClicked = false;
    }
}

void createWindow(const char* title, int width, int height) {
    window.title = title;
    window.width = width;
    window.height = height;

    window.dt = 0.0f;
    window.lastFrame = 0.0f;

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwSwapInterval(1);

    #ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

    window.self = glfwCreateWindow(window.width, window.height, window.title, NULL, NULL);
    if (window.self == NULL) {
        ERROR("Failed to create GLFW window!");
        glfwTerminate();
        exit(1);
    }

    glfwMakeContextCurrent(window.self);

    //Load all OpenGL function pointers with Glad
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        ERROR("Failed to initialize GLAD!");
        exit(1);
    }

    glEnable(GL_DEPTH_TEST); 

    glfwSetFramebufferSizeCallback(window.self, _sizeCallback);
    glfwSetKeyCallback(window.self, _keyboardCallback);
    glfwSetCursorPosCallback(window.self, _mouseCallback);
    glfwSetMouseButtonCallback(window.self, _mouseButtonCallback);

    glfwSetInputMode(window.self, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    LOG("Window succesfully initilized!");
}

void terminateWindow() {
    glfwDestroyWindow(window.self);
    glfwTerminate();
}

void updateWindow() {    

    float currentFrame = glfwGetTime();
    window.dt = currentFrame - window.lastFrame;
    window.lastFrame = currentFrame;

    // render
    glClearColor(0.4f, 0.9f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
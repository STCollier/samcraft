#include "window.h"

struct Window window;

static void _sizeCallback() {
    int width, height;
    glfwGetWindowSize(window.self, &width, &height);

    glViewport(0, 0, width, height);
}

static void _keyboardCallback(GLFWwindow* w, int key, int scancode, int action, int mods) {
    if (glfwGetKey(window.self, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window.self, 1);

    if (glfwGetKey(window.self, GLFW_KEY_TAB) == GLFW_PRESS) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    if (action == GLFW_PRESS) {
        window.keyPressed[key] = true;
    } else if (action == GLFW_RELEASE) {
        window.keyPressed[key] = false;
    }
}

static void _mouseCallback(GLFWwindow* w, double xpos, double ypos) {
    camera_mouseCallback(xpos, ypos);

    glfwGetCursorPos(window.self, &window.mouseX, &window.mouseY);
}

static void _mouseButtonCallback(GLFWwindow* w, int button, int action, int mods) {
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

    if (action == GLFW_RELEASE) window.onMouseRelease = true;
    else window.onMouseRelease = false;
}

void window_create(const char* title, int width, int height) {
    window.title = title;
    window.width = width;
    window.height = height;

    window.dt = 0.0f;
    window.lastFrame = 0.0f;

    window.leftClicked = false;
    window.rightClicked = false;
    window.onMouseRelease = true;
  
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    #ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

    window.self = glfwCreateWindow(window.width, window.height, window.title, NULL, NULL);

    if (window.self == NULL) {
        printf("Failed to create GLFW window");
        glfwTerminate();
        exit(1);
    }

    glfwMakeContextCurrent(window.self);

    // Load all OpenGL function pointers with Glad
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        printf("Failed to initialize GLAD");
        exit(1);
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glfwSetFramebufferSizeCallback(window.self, _sizeCallback);
    glfwSetKeyCallback(window.self, _keyboardCallback);
    glfwSetCursorPosCallback(window.self, _mouseCallback);
    glfwSetMouseButtonCallback(window.self, _mouseButtonCallback);

    glfwSetInputMode(window.self, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwSwapInterval(1);
}

void window_destroy() {
    glfwDestroyWindow(window.self);
    glfwTerminate();
}

void window_update() {

    float currentFrame = glfwGetTime();
    window.dt = currentFrame - window.lastFrame;
    window.lastFrame = currentFrame;

    // Render
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
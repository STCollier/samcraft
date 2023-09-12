#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include "cglm/cglm.h"
#include "cglm/call.h"

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "game/window.h"
#include "game/shader.h"
#include "game/camera.h"

#include "world/chunk.h"
#include "world/world.h"

int main() {

    createWindow("Minecraft", 1920, 1080);

    struct Shader mainShader = createShader("res/shaders/main.vs", "res/shaders/main.fs");
    initCamera(30.0f, 10.0f, 0.1f);

    loadWorld();

    while (!glfwWindowShouldClose(window.self)) {
        updateWindow();

        useShader(mainShader);
        useCamera(mainShader);

        renderWorld(mainShader);

        glfwSwapBuffers(window.self);
        glfwPollEvents();
    }

    terminateWindow();
   
    return 0;
}
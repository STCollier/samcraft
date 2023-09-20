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
    initCamera(30.0f, 25.0f, 0.1f);

    loadBlocksFromFile();
    loadArrayTexture();
    loadWorld();

    int tick = 0;
    while (!glfwWindowShouldClose(window.self)) {
        updateWindow();

        tick++;
        if (tick > 30) {
            printf("FPS: %f\n", 1/window.dt);
            tick = 0;
        }
        


        useShader(mainShader);
        useCamera(mainShader);

        renderWorld(mainShader);

        glfwSwapBuffers(window.self);
        glfwPollEvents();
    }

    free(world.chunks);
    terminateWindow();
   
    return 0;
}
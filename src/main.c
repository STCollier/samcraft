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
#include "game/serialize.h"
#include "game/ray.h"
#include "game/player.h"

#include "world/chunk.h"
#include "world/world.h"
#include "world/worldgen.h"

#define SEED 123456789

int main() {
    createWindow("Minecraft", 1920, 1080);
    stbi_set_flip_vertically_on_load(true);

    struct Shader mainShader = createShader("res/shaders/main.vs", "res/shaders/main.fs");
    initCamera(30.0f, 25.0f, 0.1f);

    initWorld();

    worldgenInit(SEED);
    initWorldDB(SEED);

    loadBlocksFromFile();
    loadArrayTexture();
    loadWorld();

    initPlayer();

    struct Ray cameraRay = ray(camera.position, camera.front, 3.0f);

    int tick = 0;
    while (!glfwWindowShouldClose(window.self)) {
        updateWindow();

        updateRay(&cameraRay, camera.position, camera.front);
        updatePlayer();

        tick++;
        if (tick > 30) {
            printf("FPS: %f\n", 1/window.dt);
            tick = 0;
        }

        if (player.movedBetweenChunks) {
            moveWorld(player.chunkPos);

            player.movedBetweenChunks = false;
        }

        useShader(mainShader);
        useCamera(mainShader);

        renderWorld(mainShader);

        glfwSwapBuffers(window.self);
        glfwPollEvents();
    }

    //  destroyWorld();
    terminateWindow();
   
    return 0;
}
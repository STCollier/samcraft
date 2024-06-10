#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include "engine/core/window.h"
#include "engine/core/resources.h"

#include "engine/util/timer.h"

int main() {
    window_create("samcraft", 1600, 900);
    resources_load();

    while (!glfwWindowShouldClose(window.self)) {
        window_update();

        resources_render();
        resources_update();

        glfwSwapBuffers(window.self);
        glfwPollEvents();
    }

    resources_destroy();
    window_destroy();

    return 0;
}
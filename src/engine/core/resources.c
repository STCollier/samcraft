#include <stb/stb_image.h>

#include "resources.h"
#include "window.h"
#include "camera.h"
#include "globals.h"

#include "../gfx/ui.h"
#include "../gfx/light.h"
#include "../gfx/postprocess.h"
#include "../../world/block.h"
#include "../../world/chunk.h"
#include "../../world/world.h"

static struct Resources res;

void resources_load() {
    globals_init();

    // Set objects
    res.shaders.main = shader_new("res/shaders/main.vert", "res/shaders/main.frag");
    res.shaders.blockOverlay = shader_new("res/shaders/block-overlay.vert", "res/shaders/block-overlay.frag");
    res.shaders.sky = shader_new("res/shaders/sky.vert", "res/shaders/sky.frag");
    res.shaders.sprite = shader_new("res/shaders/sprite.vert", "res/shaders/sprite.frag");
    res.shaders.text = shader_new("res/shaders/text.vert", "res/shaders/text.frag");
    res.shaders.depth = shader_new("res/shaders/depth.vert", "res/shaders/depth.frag");
    res.shaders.hdr = shader_new("res/shaders/hdr.vert", "res/shaders/hdr.frag");

    mat4 projection2D;
    glm_ortho(0.0f, window.width, window.height, 0.0f, -1.0f, 1.0f, projection2D);

    shader_use(res.shaders.text);
    shader_setMat4(res.shaders.text, "projection", projection2D);
    shader_use(res.shaders.sprite);
    shader_setMat4(res.shaders.sprite, "projection", projection2D);

    stbi_set_flip_vertically_on_load(true);
    ui_init();
    blockdata_loadLuaData();
    blockdata_loadMaterials(res.shaders.main);
    world_init(globals.renderRadius);
    player_init();
    light_init();
    HDR_init();

    res.text.FPS = ui_newText("FPS: 120", 50.0, 75.0, 0.4, (ivec4){0, 0, 0, 255});
    res.text.playerPosition = ui_newText("Position: (0, 0, 0)", 50.0, 200.0, 0.4, (ivec4){0, 0, 0, 255});
    ui_set(TEXT_LAYER);

    res.sprites.crosshair = ui_newSprite("res/textures/ui/crosshair.png", window.width / 2, window.height / 2, 2.0, INHERIT, CLEAR_COLOR, ALIGN_CENTER);
    res.sprites.textPanel = ui_newSprite("res/textures/ui/panel.png", 25, 25, 1.0, (vec2){600, 125}, CLEAR_COLOR, ALIGN_LEFT);
    ui_set(SPRITE_LAYER);

    res.skybox = skybox_new();
    res.thpool = thpool_init(globals.threads);

    res.timers.FPSTimer = timer_new(1.0);
    res.timers._100ms = timer_new(0.1);
    res.timers._500ms = timer_new(0.5);
}

static char FPSt[64];
static char playerPositiont[128];
void resources_update() {
    camera_use(res.shaders.main);
    world_update(res.thpool);
    res.cameraFrustum = updateCameraFrustum();

    timer_update(&res.timers.FPSTimer);
    if (res.timers.FPSTimer.ended) {
        snprintf(FPSt, 64, "FPS: %d", window.FPS);
        ui_updateText(res.text.FPS, FPSt, res.text.FPS.x, res.text.FPS.y, res.text.FPS.scale, res.text.FPS.color);
        timer_reset(&res.timers.FPSTimer);
    }

    timer_update(&res.timers._100ms);
    if (res.timers._100ms.ended) {
        snprintf(playerPositiont, 128, "Position: (%.2f %.2f %.2f)", camera.position[0], camera.position[1], camera.position[2]);
        ui_updateText(res.text.playerPosition, playerPositiont, res.text.playerPosition.x, res.text.playerPosition.y, res.text.playerPosition.scale, res.text.playerPosition.color);

        ui_set(TEXT_LAYER);
        timer_reset(&res.timers._100ms);
    }
}

void resources_render() {
    light_beginPass(res.shaders.depth);

    world_render(res.shaders.depth, res.cameraFrustum, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, window.width, window.height);

    HDR_set();
    shader_use(res.shaders.main);

    if (glfwGetKey(window.self, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        light.sunPosition[0] += window.dt * 250;
        light.sunPosition[2] += window.dt * 250;
    }
    if (glfwGetKey(window.self, GLFW_KEY_LEFT) == GLFW_PRESS) {
        light.sunPosition[0] -= window.dt * 250;
        light.sunPosition[2] -= window.dt * 250;
    }

    // Set shader uniforms
    shader_setMat4(res.shaders.main, "light_space_matrix", light.spaceMatrix);
    shader_setVec3(res.shaders.main, "sun_position", light.sunPosition[0], light.sunPosition[1], light.sunPosition[2]);
    shader_setVec3(res.shaders.main, "camera_position", camera.position[0], camera.position[1], camera.position[2]);

    shader_setFloat(res.shaders.main, "fog_max", ((world.renderRadius - 1) * CHUNK_SIZE) - CHUNK_SIZE / 2);
    shader_setFloat(res.shaders.main, "fog_min", (world.renderRadius / 2) * CHUNK_SIZE);

    skybox_render(res.skybox, res.shaders.sky);
    world_render(res.shaders.main, res.cameraFrustum, 1);
    player_update(res.shaders.blockOverlay);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    HDR_use(res.shaders.hdr);
    
    // Draw UI
    glDisable(GL_DEPTH_TEST);

    ui_render(SPRITE_LAYER, res.shaders.sprite);
    ui_render(TEXT_LAYER, res.shaders.text);

    glEnable(GL_DEPTH_TEST);
}

void resources_destroy() {
    thpool_destroy(res.thpool);
}
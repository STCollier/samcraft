#include <stb/stb_image.h>

#include "resources.h"
#include "window.h"
#include "camera.h"
#include "globals.h"

#include "../gfx/ui.h"
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

    shader_setUniformBlock(res.shaders.main, "CameraMatrix", 0);

    mat4 projection2D;
    glm_ortho(0.0f, window.width, window.height, 0.0f, -1.0f, 1.0f, projection2D);

    shader_use(res.shaders.text);
    shader_setMat4(res.shaders.text, "projection", projection2D);
    shader_use(res.shaders.sprite);
    shader_setMat4(res.shaders.sprite, "projection", projection2D);

    stbi_set_flip_vertically_on_load(true);
    ui_init();
    blockdata_loadLuaData();
    blockdata_loadArrayTexture();
    world_init(globals.renderRadius);
    player_init();

    res.text.frameTime = ui_newText("Frame Time: 0 ms", 50.0, 75.0, 0.4, (ivec4){0, 0, 0, 255});
    res.text.FPS = ui_newText("FPS: 0", 50.0, 200.0, 0.4, (ivec4){0, 0, 0, 255});
    res.text.playerPosition = ui_newText("Position: (0, 0, 0)", 50.0, 325.0, 0.4, (ivec4){0, 0, 0, 255});
    ui_set(TEXT_LAYER);

    res.sprites.crosshair = ui_newSprite("res/textures/ui/crosshair.png", window.width / 2, window.height / 2, 2.0, INHERIT, CLEAR_COLOR, ALIGN_CENTER);
    res.sprites.textPanel = ui_newSprite("res/textures/ui/panel.png", 25, 25, 1.0, (vec2){600, 175}, CLEAR_COLOR, ALIGN_LEFT);
    ui_set(SPRITE_LAYER);

    res.skybox = skybox_new();
    res.thpool = thpool_init(globals.threads);

    res.timers.FPSTimer = timer_new(1.0);
    res.timers._100ms = timer_new(0.1);
    res.timers._500ms = timer_new(0.5);
}

static char FPSt[64];
static char frameTimet[64];
static char playerPositiont[128];
static float averageFrameTime = 0.0;

void resources_update() {
    camera_use(res.shaders.main);
    player_update(res.shaders.blockOverlay);
    res.cameraFrustum = updateCameraFrustum();

    timer_update(&res.timers.FPSTimer);
    averageFrameTime += window.dt * 1000;

    if (res.timers.FPSTimer.ended) {
        snprintf(FPSt, 64, "FPS: %d", window.FPS);
        ui_updateText(res.text.FPS, FPSt, res.text.FPS.x, res.text.FPS.y, res.text.FPS.scale, res.text.FPS.color);

        snprintf(frameTimet, 64, "Frame Time: %.2f ms", window.dt * 1000);
        ui_updateText(res.text.frameTime, frameTimet, res.text.frameTime.x, res.text.frameTime.y, res.text.frameTime.scale, res.text.frameTime.color);

        averageFrameTime = 0;

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
    skybox_render(res.skybox, res.shaders.sky);
    world_render(res.shaders.main, res.thpool, res.cameraFrustum);

    // Set shader uniforms
    shader_setVec3(res.shaders.main, "camera_position", camera.position[0], camera.position[1], camera.position[2]);
    shader_setVec3(res.shaders.main, "camera_direction", camera.front[0], camera.front[1], camera.front[2]);

    shader_setFloat(res.shaders.main, "fog_max", ((world.renderRadius - 1) * CHUNK_SIZE) - CHUNK_SIZE / 2);
    shader_setFloat(res.shaders.main, "fog_min", (world.renderRadius / 2) * CHUNK_SIZE);

    // Draw UI
    glDisable(GL_DEPTH_TEST);

    ui_render(SPRITE_LAYER, res.shaders.sprite);
    ui_render(TEXT_LAYER, res.shaders.text);

    glEnable(GL_DEPTH_TEST);
}

void resources_destroy() {
    thpool_destroy(res.thpool);
}
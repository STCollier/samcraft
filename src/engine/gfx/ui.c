#include "ui.h"

struct Ui ui;

void ui_init() {
    ui.robotoFont = font_load("res/fonts/roboto.ttf", 100);
    ui.spriteData = sprite_loadAtlas();

    ui.textMesh = quadmesh_init();
    ui.spriteMesh = quadmesh_init();

    ui.textList = text_t_array();
    ui.spriteList = sprite_t_array();
}

struct Text ui_newText(const char* string, float x, float y, float scale, ivec4 color) {
    struct Text text = {
        .string = string,
        .x = x,
        .y = y,
        .scale = scale,
        .color = {color[0], color[1], color[2], color[3]}
    };

    text_t_arr_push(&ui.textList, text);
    text_new(&ui.textMesh, &ui.robotoFont, text);

    return text;
}

struct Sprite ui_newSprite(const char* filename, float x, float y, float scale, vec2 dimensions, ivec4 color, AlignMode align) {
    struct Sprite sprite = {
        .filename = filename,
        .x = x,
        .y = y,
        .scale = scale,
        .align = align,
        .dimensions = {dimensions[0], dimensions[1]},
        .color = {color[0], color[1], color[2], color[3]}
    };

    sprite_t_arr_push(&ui.spriteList, sprite);
    sprite_new(&ui.spriteMesh, ui.spriteData, sprite, align);

    return sprite;
}

void ui_set(enum UiLayer layer) {
    layer == TEXT_LAYER ? 
        quadmesh_bind(&ui.textMesh) :
        quadmesh_bind(&ui.spriteMesh);
}

void ui_updateText(struct Text t, const char* text, float x, float y, float scale, ivec4 color) {
    quadmesh_clear(&ui.textMesh);

    for (int i = 0; i < ui.textList.length; i++) {
        struct Text textItem = ui.textList.data[i];

        // Find text object within ui.textList
        if (strcmp(ui.textList.data[i].string, t.string) == 0 && ui.textList.data[i].x == t.x && ui.textList.data[i].y == t.y) {
            ui.textList.data[i] = (struct Text){
                .string = text,
                .x = x,
                .y = y,
                .scale = scale,
                .color = {color[0], color[1], color[2], color[3]}
            };
        }

        text_new(&ui.textMesh, &ui.robotoFont, textItem);
    }
}

void ui_render(enum UiLayer layer, shader_t shader) {
    layer == TEXT_LAYER ? 
        quadmesh_render(&ui.textMesh, shader, ui.robotoFont.textureID) : 
        quadmesh_render(&ui.spriteMesh, shader, ui.spriteData.textureID);
}

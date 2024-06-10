#ifndef UI_H
#define UI_H

#include "../util/common.h"
#include "text.h"
#include "sprite.h"
#include "quad.h"

typedef struct Text text_t;
typedef struct Sprite sprite_t;

DEFINE_ARRAY_IMPL(text_t);
DEFINE_ARRAY_IMPL(sprite_t);

enum UiLayer {
    TEXT_LAYER,
    SPRITE_LAYER
};

struct Ui {
    struct Font robotoFont;
    struct SpriteData spriteData;
    struct QuadMesh textMesh, spriteMesh;

    arr_text_t textList;
    arr_sprite_t spriteList;
};

void ui_init();

struct Text ui_newText(const char* string, float x, float y, float scale, ivec4 color);
struct Sprite ui_newSprite(const char* filename, float x, float y, float scale, vec2 dimensions, ivec4 color, AlignMode align);

void ui_set(enum UiLayer layer);
void ui_updateText(struct Text t, const char* text, float x, float y, float scale, ivec4 color);
void ui_render(enum UiLayer layer, shader_t shader);

extern struct Ui ui;

#endif
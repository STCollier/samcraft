#ifndef TEXT_H
#define TEXT_H

#include "../util/common.h"
#include "../core/shader.h"
#include "quad.h"
#include "image.h"

struct Character {
    size_t offset; // Offset in font spritesheet
    ivec2 size; // Size of glyph
    ivec2 bearing; // Offset from baseline to left/top of glyph
    unsigned int advance; // Offset to advance to next glyph
};

struct Text {
    const char* string;
    float x, y, scale;
    ivec4 color;
};

struct Font {
    struct Character characters[256];
    struct Image spritesheet;
    unsigned int textureID;
    const char* src;
    unsigned int size;
};

struct Font font_load(const char* src, unsigned int size);
void text_new(struct QuadMesh* qm, struct Font* font, struct Text text);

#endif
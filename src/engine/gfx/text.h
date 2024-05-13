#ifndef TEXT_H
#define TEXT_H

#include "../util/common.h"

struct Character {
    unsigned int textureID; // ID handle of the glyph texture
    ivec2 size; // Size of glyph
    ivec2 bearing; // Offset from baseline to left/top of glyph
    unsigned int advance; // Offset to advance to next glyph
};

struct Font {
    struct Character characters[256];
    unsigned int VBO, VAO;
    const char* src;
    unsigned int size;
};

struct Font font_load(const char* src, unsigned int size);
void text_render(struct Font* font, shader_t shader, char* text, float x, float y, float scale, vec3 color);

#endif
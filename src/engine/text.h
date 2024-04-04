#ifndef TEXT_H
#define TEXT_H

#include <stdlib.h>
#include <string.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "cglm/cglm.h"
#include "cglm/call.h"
#include "shader.h"
#include "util.h"

struct Font {
    const char* glyphs;
    const char* bitmap;
    size_t glyphSize, numGlyphs;
    shader_t shader;

    unsigned int bitmapArrayTexture;
};

struct Character {
    ivec2 position;
    unsigned int VBO, VAO;
    size_t charIndex;
};

struct Text {
    struct Font font;
    ivec2 position;
    char* string;
    size_t length;

    struct Character *characters;
};

struct Font font_load(const char* bitmap, const char* glyphs, size_t glyphSize, shader_t shader);
struct Text text_new(char* string, ivec2 position, struct Font font);
void text_update(struct Text text, char* newString);
void text_draw(struct Text text);

#endif
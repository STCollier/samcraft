#ifndef SPRITE_H
#define SPRITE_H

#include "../util/common.h"

struct Sprite2D {
    const char* textureName;
    ivec2 position;
    vec2 dimensions;
    float scale;

    unsigned int VBO, VAO, textureID;
    unsigned char* textureData;
};

struct Sprite2D sprite2D_new(const char* filename, ivec2 position, float scale);
void sprite2D_render(struct Sprite2D *sprite, AlignMode align, shader_t shader);

#endif
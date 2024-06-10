#ifndef SPRITE_H
#define SPRITE_H

#include "../util/common.h"
#include "image.h"
#include "quad.h"

struct SpriteLookup {
    struct Image texture;
    char src[64];
    vec2 uv[4];
};

struct SpriteData {
    struct Image textureAtlas;
    unsigned int textureID;
    struct SpriteLookup* spriteLookup;
    size_t numSprites;
};

struct Sprite {
    const char* filename;
    float x, y, scale;
    AlignMode align;
    size_t offset;
    vec2 dimensions;
    ivec4 color;
};

struct SpriteData sprite_loadAtlas();
void sprite_new(struct QuadMesh* qm, struct SpriteData spriteData, struct Sprite sprite, AlignMode align);

#endif
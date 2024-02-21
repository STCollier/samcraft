#ifndef SPRITE_2D_H
#define SPRITE_2D_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "cglm/cglm.h"
#include "cglm/call.h"

#include "util.h"
#include "shader.h"

struct Sprite2D {
    const char* textureName;
    ivec2 position;
    float scale;

    unsigned int VBO, VAO, textureID;
    unsigned char* textureData;
};

struct Sprite2D sprite2D_new(const char* filename, ivec2 position, float scale);
void sprite2D_render(struct Sprite2D *sprite, shader_t shader);

#endif
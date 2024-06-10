#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb/stb_image.h>
#include <dirent.h>

#include "../core/shader.h"

#include "sprite.h"

struct SpriteData sprite_loadAtlas() {
    struct SpriteData spriteData;

    struct dirent *dir;
    size_t len = 0;

    DIR *dcount = opendir("res/textures/ui");

    if (dcount) {
        while ((dir = readdir(dcount)) != NULL) {
            if (dir->d_type == DT_DIR || strcmp(strrchr(dir->d_name, '.') + 1, "png") != 0) continue;
            len++;
        }
        closedir(dcount);
    }

    spriteData.numSprites = len;

    DIR *dload = opendir("res/textures/ui");

    struct Image sprites[len];
    spriteData.spriteLookup = malloc(len * sizeof(struct SpriteLookup));

    if (dload) {
        int i = 0;
        while ((dir = readdir(dload)) != NULL) {
            if (dir->d_type == DT_DIR || strcmp(strrchr(dir->d_name, '.') + 1, "png") != 0) continue;
            char path[64] = "res/textures/ui/";
            strcat(path, dir->d_name);

            sprites[i] = image_new(path);
            spriteData.spriteLookup[i].texture = sprites[i];
            strcpy(spriteData.spriteLookup[i].src, path);

            i++;
        }
        closedir(dload);
    }

    spriteData.textureAtlas = data_spritesheet_new(len, sprites);

    size_t offset = 0;
    for (int i = 0; i < len; i++) {
        float uvx = offset == 0 ? 0.0 : (float) offset / spriteData.textureAtlas.width;
        vec2 uvoffset = {(float) sprites[i].width / spriteData.textureAtlas.width, (float) sprites[i].height / spriteData.textureAtlas.height};

        vec2 uv[4] = {
            {uvx, 0.0},
            {uvx + uvoffset[0], 0.0},
            {uvx + uvoffset[0], uvoffset[1]},
            {uvx, uvoffset[1]}
        };

        for (int j = 0; j < 4; j++) glm_vec2_copy(uv[j], spriteData.spriteLookup[i].uv[j]);
    
        offset += sprites[i].width + 1; // + 1 pixel padding
    }

    glGenTextures(1, &spriteData.textureID);
    glBindTexture(GL_TEXTURE_2D, spriteData.textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(
        GL_TEXTURE_2D, 
        0, 
        GL_SRGB8_ALPHA8,
        spriteData.textureAtlas.width, 
        spriteData.textureAtlas.height, 
        0, 
        GL_RGBA, 
        GL_UNSIGNED_BYTE, 
        spriteData.textureAtlas.texture
    );

    return spriteData;
}

struct SpriteLookup sprite_lookup(struct SpriteData spriteData, const char src[64]) {
    struct SpriteLookup sprite;

    bool found = false;
    for (int i = 0; i < spriteData.numSprites; i++) {
        if (strcmp(spriteData.spriteLookup[i].src, src) == 0) {
            strcpy(sprite.src, src);
            sprite.texture = spriteData.spriteLookup[i].texture;
            for (int j = 0; j < 4; j++) glm_vec2_copy(spriteData.spriteLookup[i].uv[j], sprite.uv[j]);
            found = true; break;
        }
    }

    if (found) {
        return sprite;
    } else {
        ERROR_MSG("Failed to find sprite", src);
        exit(EXIT_FAILURE);
    }
}

void sprite_new(struct QuadMesh* qm, struct SpriteData spriteData, struct Sprite sprite, AlignMode align) {
    struct SpriteLookup sl = sprite_lookup(spriteData, sprite.filename);

    if (glm_vec2_eq(sprite.dimensions, -1)) {
        sprite.dimensions[0] = sl.texture.width;
        sprite.dimensions[1] = sl.texture.height;
    }

    vec2 dimensions = {sprite.dimensions[0] * sprite.scale, sprite.dimensions[1] * sprite.scale};

    if (align == ALIGN_LEFT) {
        // Already left aligned
    } else if (align == ALIGN_RIGHT) {
        sprite.x = sprite.x - sprite.dimensions[0] * sprite.scale;
        sprite.y = sprite.y - sprite.dimensions[1] * sprite.scale;
    } else { // Center
        sprite.x = sprite.x - (sprite.dimensions[0] / 2) * sprite.scale;
        sprite.y = sprite.y - (sprite.dimensions[1] / 2) * sprite.scale;
    }

    quad_add(qm, (vec2){sprite.x, sprite.y}, dimensions, sl.uv, sprite.color, sprite.scale);
}
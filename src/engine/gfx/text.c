#include <freetype/freetype.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "text.h"

struct Font font_load(const char* src, unsigned int size) {
    struct Font font;

    font.src = src;
    font.size = size;

    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        ERROR("Could not init FreeType Library");
        exit(EXIT_FAILURE);
    }

    FT_Face face;
    if (FT_New_Face(ft, font.src, 0, &face)) {
        ERROR("Could not load font");
        exit(EXIT_FAILURE);
    }

    FT_Set_Pixel_Sizes(face, 0, font.size);

    struct Image glyphSpritesheetData[128];
    size_t xoffset = 0;
    for (unsigned char c = 0; c < 128; c++) {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            ERROR_IMSG("Failed to load glyph", c);
            continue;
        }

        struct Image g = {
            .channels = 1,
            .width = face->glyph->bitmap.width,
            .height = face->glyph->bitmap.rows,
            .size = face->glyph->bitmap.width * face->glyph->bitmap.rows,
            .texture = malloc(face->glyph->bitmap.width * face->glyph->bitmap.rows)
        };

        memcpy(g.texture, face->glyph->bitmap.buffer, g.size);
        glyphSpritesheetData[c] = g;

        struct Character character;
        glm_ivec2_copy((ivec2){face->glyph->bitmap.width, face->glyph->bitmap.rows}, character.size);
        glm_ivec2_copy((ivec2){face->glyph->bitmap_left, face->glyph->bitmap_top}, character.bearing);
        character.offset = xoffset;
        character.advance = face->glyph->advance.x;

        font.characters[c] = character;

        xoffset += face->glyph->bitmap.width + 1; // + 1 pixel padding
    }

    font.spritesheet = data_spritesheet_new(128, glyphSpritesheetData);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glGenTextures(1, &font.textureID);
    glBindTexture(GL_TEXTURE_2D, font.textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RED,
        font.spritesheet.width,
        font.spritesheet.height,
        0,
        GL_RED,
        GL_UNSIGNED_BYTE,
        font.spritesheet.texture
    );

    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    LOG_MSG("Loaded font", font.src);

    return font;
}

void text_new(struct QuadMesh* qm, struct Font* font, struct Text text) {
    for (int c = 0; c < strlen(text.string); c++) {
        struct Character ch = font->characters[*(text.string + c)];

        vec2 dimensions = {ch.size[0] * text.scale, ch.size[1] * text.scale};
        vec2 pos = {
            text.x + ch.bearing[0] * text.scale,
            (text.y - ch.bearing[1] + font->spritesheet.height) * text.scale
        };

        float uvx = ch.offset == 0 ? 0.0 : (float) ch.offset / font->spritesheet.width;
        vec2 uvoffset = {(float) ch.size[0] / font->spritesheet.width, (float) ch.size[1] / font->spritesheet.height}; // There's no implict float casting aaa

        vec2 uv[4] = {
            {uvx, 0.0},
            {uvx + uvoffset[0], 0.0},
            {uvx + uvoffset[0], uvoffset[1]},
            {uvx, uvoffset[1]}
        };

        quad_add(qm, pos, dimensions, uv, text.color, text.scale);

        text.x += (ch.advance >> 6) * text.scale;
    }
}
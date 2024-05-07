#include "text.h"
#include <freetype/freetype.h>

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
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    for (unsigned char c = 0; c < 128; c++) {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            ERROR_IMSG("Failed to load glyph", c);
            continue;
        }

        unsigned int glyphTexture;
        glGenTextures(1, &glyphTexture);
        glBindTexture(GL_TEXTURE_2D, glyphTexture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        //glGenerateMipmap(GL_TEXTURE_2D);

        struct Character character;
        character.textureID = glyphTexture;
        glm_ivec2_copy((ivec2){face->glyph->bitmap.width, face->glyph->bitmap.rows}, character.size);
        glm_ivec2_copy((ivec2){face->glyph->bitmap_left, face->glyph->bitmap_top}, character.bearing);
        character.advance = face->glyph->advance.x;

        font.characters[c] = character;
    }

    glBindTexture(GL_TEXTURE_2D, 0);

    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    glGenVertexArrays(1, &font.VAO);
    glGenBuffers(1, &font.VBO);
    glBindVertexArray(font.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, font.VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    LOG_MSG("Loaded font", font.src);

    return font;
}

void text_render(struct Font* font, shader_t shader, char* text, float x, float y, float scale, vec3 color) {
    shader_use(shader);
    shader_setVec3(shader, "textColor", color[0], color[1], color[2]);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(font->VAO);

    for (int c = 0; c < strlen(text); c++) {
        struct Character ch = font->characters[*(text + c)];

        float xpos = x + ch.bearing[0] * scale;
        float ypos = y - (ch.size[1] - ch.bearing[1]) * scale;

        float w = ch.size[0] * scale;
        float h = ch.size[1] * scale;

        float vertices[6][4] = {
            { xpos, ypos + h, 0.0f, 0.0f },            
            { xpos, ypos, 0.0f, 1.0f },
            { xpos + w, ypos, 1.0f, 1.0f },

            { xpos, ypos + h, 0.0f, 0.0f },
            { xpos + w, ypos, 1.0f, 1.0f },
            { xpos + w, ypos + h, 1.0f, 0.0f }           
        };

        // render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.textureID);

        // update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, font->VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); 

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)
    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}
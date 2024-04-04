#include "text.h"
#include <stb/stb_image.h>

/* 
 ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz01234567890`~!@#$%^&*()-_+=[]{}\|;:'"<>,./?
https://www.dafont.com/perfect-dos-vga-437.font
https://stmn.itch.io/font2bitmap
*/

static const float vertices[] = { 
    // pos      // tex
    0.0f, 1.0f, 0.0f, 1.0f,
    1.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, 

    0.0f, 1.0f, 0.0f, 1.0f,
    1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, 0.0f, 1.0f, 0.0f
};

static void getGlyphTexture(unsigned char *dataOut, unsigned char *dataIn, size_t bitmapWidth, size_t glyphSize, size_t index) {
    for (size_t y = 0; y < glyphSize; y++) {
        for (size_t x = 0; x < glyphSize; x++) {
            dataOut[4 * ((glyphSize * y) + x) + 0] = dataIn[4 * ( bitmapWidth * y + glyphSize * index + x ) + 0];
            dataOut[4 * ((glyphSize * y) + x) + 1] = dataIn[4 * ( bitmapWidth * y + glyphSize * index + x ) + 1];
            dataOut[4 * ((glyphSize * y) + x) + 2] = dataIn[4 * ( bitmapWidth * y + glyphSize * index + x ) + 2];
            dataOut[4 * ((glyphSize * y) + x) + 3] = dataIn[4 * ( bitmapWidth * y + glyphSize * index + x ) + 3];
        }
    }
}

struct Font font_load(const char* bitmap, const char* glyphs, size_t glyphSize, shader_t shader) {
    struct Font font;

    font.glyphs = glyphs;
    font.bitmap = bitmap;
    font.glyphSize = glyphSize;
    font.numGlyphs = strlen(glyphs);
    font.shader = shader;

    GL_CHECK(glGenTextures(1, &font.bitmapArrayTexture));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D_ARRAY, font.bitmapArrayTexture));

    GL_CHECK(glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT));    
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT));

    GL_CHECK(glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST));

    // Allocate texture storage with GL_RGBA8 internal format
    GL_CHECK(glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_SRGB8_ALPHA8, font.glyphSize, font.glyphSize, font.numGlyphs, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL));

    int width, height, channels;
    unsigned char* bitmapData = stbi_load(font.bitmap, &width, &height, &channels, 4);

    if (!bitmapData) {
        ERROR_MSG("Failed to load font bitmap texture at path", font.bitmap);
        exit(EXIT_FAILURE);
    } else {
        LOG_MSG("Loaded font bitmap texture", font.bitmap);
    }

    const size_t size = font.glyphSize * font.glyphSize * 4;
    for (int i = 0; i < font.numGlyphs; i++) {
        unsigned char* pixels = malloc(size);
        getGlyphTexture(pixels, bitmapData, font.numGlyphs * glyphSize, glyphSize, i);

        GL_CHECK(glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, font.glyphSize, font.glyphSize, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixels));

        free(pixels);
    }

    GL_CHECK(glGenerateMipmap(GL_TEXTURE_2D_ARRAY));
    stbi_image_free(bitmapData);

    return font;
}

struct Text text_new(char* string, ivec2 position, struct Font font) {
    struct Text text;

    text.string = string;
    text.font = font;
    text.length = strlen(text.string);
    glm_ivec2_copy(position, text.position);

    text.characters = malloc(sizeof(struct Character) * text.length);

    for (int i = 0; i < text.length; i++) {
        text.characters[i].position[0] = text.position[0] + i * text.font.glyphSize;
        text.characters[i].position[1] = text.position[1];
        
        int idx = -1;
        for (int j = 0; j < text.font.numGlyphs; j++) {
            if (text.string[i] == text.font.glyphs[j]) {
                idx = j;
            }
        }

        if (idx == -1) idx = text.font.numGlyphs;

        text.characters[i].charIndex = idx;

        GL_CHECK(glGenVertexArrays(1, &text.characters[i].VAO));
        GL_CHECK(glGenBuffers(1, &text.characters[i].VBO));

        GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, text.characters[i].VBO));
        GL_CHECK(glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW));

        GL_CHECK(glBindVertexArray(text.characters[i].VAO));
        GL_CHECK(glEnableVertexAttribArray(0));
        GL_CHECK(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0));
        GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));  
        GL_CHECK(glBindVertexArray(0));
    }


    return text;
}

void text_update(struct Text text, char* newString) {
    text.string = newString;
    text.length = strlen(text.string);

    text.characters = realloc(text.characters, sizeof(struct Character) * text.length);

    for (int i = 0; i < text.length; i++) {
        text.characters[i].position[0] = text.position[0] + i * text.font.glyphSize;
        text.characters[i].position[1] = text.position[1];
        
        int idx = -1;
        for (int j = 0; j < text.font.numGlyphs; j++) {
            if (text.string[i] == text.font.glyphs[j]) {
                idx = j;
            }
        }

        if (idx == -1) idx = text.font.numGlyphs;

        text.characters[i].charIndex = idx;

        GL_CHECK(glGenVertexArrays(1, &text.characters[i].VAO));
        GL_CHECK(glGenBuffers(1, &text.characters[i].VBO));

        GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, text.characters[i].VBO));
        GL_CHECK(glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW));

        GL_CHECK(glBindVertexArray(text.characters[i].VAO));
        GL_CHECK(glEnableVertexAttribArray(0));
        GL_CHECK(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0));
        GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));  
        GL_CHECK(glBindVertexArray(0));
    }
}

void text_draw(struct Text text) {
    for (int i = 0; i < text.length; i++) {
        shader_use(text.font.shader);
        shader_setInt(text.font.shader, "fontArrayTexture", 0);
        shader_setInt(text.font.shader, "char", text.characters[i].charIndex);

        float scale = 45.0f;

        mat4 model;
        glm_mat4_identity(model);
        glm_translate(model, (vec3){text.characters[i].position[0], text.characters[i].position[1], 0.0f});
        glm_scale(model, (vec3){scale, scale, 1.0f});

        shader_setMat4(text.font.shader, "model", model);

        GL_CHECK(glActiveTexture(GL_TEXTURE0));
        GL_CHECK(glBindTexture(GL_TEXTURE_2D_ARRAY, text.font.bitmapArrayTexture));

        GL_CHECK(glBindVertexArray(text.characters[i].VAO));
        GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, 6));
        GL_CHECK(glBindVertexArray(0));
    }
}
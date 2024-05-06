#include <stb/stb_image.h>
#include "sprite2D.h"

const float vertices[] = { 
    // pos      // tex
    0.0f, 1.0f, 0.0f, 1.0f,
    1.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, 

    0.0f, 1.0f, 0.0f, 1.0f,
    1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, 0.0f, 1.0f, 0.0f
};

struct Sprite2D sprite2D_new(const char* filename, ivec2 position, float scale) {
    struct Sprite2D sprite;
    
    sprite.textureName = filename;
    glm_ivec2_copy(position, sprite.position);
    sprite.scale = scale;

    glGenTextures(1, &sprite.textureID);
    glBindTexture(GL_TEXTURE_2D, sprite.textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    int width, height, channels;
    unsigned char *data = stbi_load(sprite.textureName, &width, &height, &channels, 0);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        sprite.textureData = data;
    } else {
        ERROR_MSG("Failed to load Sprite2D texture", sprite.textureName);
    }

    stbi_image_free(data);

    glGenVertexArrays(1, &sprite.VAO);
    glGenBuffers(1, &sprite.VBO);

    glBindBuffer(GL_ARRAY_BUFFER, sprite.VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindVertexArray(sprite.VAO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);  
    glBindVertexArray(0);

    return sprite;
};

void sprite2D_render(struct Sprite2D *sprite, shader_t shader) {
    shader_use(shader);
    shader_setInt(shader, "image", 0);

    mat4 model;
    glm_mat4_identity(model);
    glm_translate(model, (vec3){sprite->position[0], sprite->position[1], 0.0f});
    glm_translate(model, (vec3){-sprite->scale / 2, -sprite->scale / 2, 0.0f});
    glm_scale(model, (vec3){sprite->scale, sprite->scale, 1.0f});

    shader_setMat4(shader, "model", model);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sprite->textureID);

    glBindVertexArray(sprite->VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include "cglm/cglm.h"
#include "cglm/call.h"

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "world/window.h"
#include "world/shader.h"
#include "world/camera.h"

#define CHUNK_SIZE_X 16
#define CHUNK_SIZE_Y 256
#define CHUNK_SIZE_Z 16

const float cubeVertices[] = {
    0, 0, 0, 0, 0, // BACK
    1, 0, 0, 1, 0,
    1, 1, 0, 1, 1,
    1, 1, 0, 1, 1,
    0, 1, 0, 0, 1,
    0, 0, 0, 0, 0,

    0, 0, 1, 0, 0, // FRONT
    1, 0, 1, 1, 0,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
    0, 1, 1, 0, 1,
    0, 0, 1, 0, 0,

    0, 1, 1, 1, 0, // LEFT
    0, 1, 0, 1, 1,
    0, 0, 0, 0, 1,
    0, 0, 0, 0, 1,
    0, 0, 1, 0, 0,
    0, 1, 1, 1, 0,

    1, 1, 1, 1, 0, // RIGHT
    1, 1, 0, 1, 1,
    1, 0, 0, 0, 1,
    1, 0, 0, 0, 1,
    1, 0, 1, 0, 0,
    1, 1, 1, 1, 0,

    0, 0, 0, 0, 1, // BOTTOM
    1, 0, 0, 1, 1,
    1, 0, 1, 1, 0,
    1, 0, 1, 1, 0,
    0, 0, 1, 0, 0,
    0, 0, 0, 0, 1,

    0, 1, 0, 0, 1, // TOP
    1, 1, 0, 1, 1,
    1, 1, 1, 1, 0,
    1, 1, 1, 1, 0,
    0, 1, 1, 0, 0,
    0, 1, 0, 0, 1
};

typedef enum {
    RIGHT,
    LEFT,
    FRONT,
    BACK,
    TOP,
    BOTTOM
} Direction;


struct Block {
    int id;
};

struct Chunk {
    struct Block block[CHUNK_SIZE_X][CHUNK_SIZE_Y][CHUNK_SIZE_Z];
    int meshSize;
    float meshData[1048576];
};

void createMeshFace(Direction dir, struct Chunk *chunk, vec3 pos) {
    int index = 0;

    switch(dir) {
        case BACK:
            index = 0;
            break;
        case FRONT:
            index = 30;
            break;
        case LEFT:
            index = 60;
            break;
        case RIGHT:
            index = 90;
            break;
        case BOTTOM:
            index = 120;
            break;
        case TOP:
            index = 150;
            break;
        default:
            fprintf(stderr, "ohno\n");
            exit(EXIT_FAILURE);
            
    }

    for (int i = 0; i < 6; i++) {
        // Positions
        chunk->meshData[chunk->meshSize] = cubeVertices[(index + i*5)] + pos[0];
        chunk->meshData[chunk->meshSize + 1] = cubeVertices[(index + i*5) + 1] + pos[1];
        chunk->meshData[chunk->meshSize + 2] = cubeVertices[(index + i*5) + 2] + pos[2];

        // UVs
        chunk->meshData[chunk->meshSize + 3] = cubeVertices[(index + i*5) + 3];
        chunk->meshData[chunk->meshSize + 4] = cubeVertices[(index + i*5) + 4];

        chunk->meshSize += 5;
    }
}

int main() {

    createWindow("Minecraft", 1920, 1080);

    struct Shader mainShader = createShader("res/shaders/main.vs", "res/shaders/main.fs");
    initCamera(30.0f, 0.1f);

    struct Chunk chunk;
    chunk.meshSize = 0;
    memset(chunk.meshData, 0, sizeof(chunk.meshData));
    memset(chunk.block, 0, sizeof(chunk.block));

    // Worldgen
    for (int x = 1; x < CHUNK_SIZE_X-1; x++) {
        for (int y = 1; y < CHUNK_SIZE_Y-1; y++) {
            for (int z = 1; z < CHUNK_SIZE_Z-1; z++) {
                chunk.block[x][y][z].id = 1;
            }
        }
    }

    // Loop through chunks, if block is NOT neighboring, check which side is not neighboring, and add mesh data for that cube face to array
    for (int x = 1; x < CHUNK_SIZE_X-1; x++) {
        for (int y = 1; y < CHUNK_SIZE_Y-1; y++) {
            for (int z = 1; z < CHUNK_SIZE_Z-1; z++) {
                if (chunk.block[x][y][z].id != 0) {
                    if (chunk.block[x-1][y][z].id == 0) createMeshFace(LEFT,   &chunk, (vec3){x, y, z}); // LEFT
                    if (chunk.block[x][y-1][z].id == 0) createMeshFace(BOTTOM, &chunk, (vec3){x, y, z}); // BOTTOM
                    if (chunk.block[x][y][z-1].id == 0) createMeshFace(BACK,   &chunk, (vec3){x, y, z}); // BACK
                    if (chunk.block[x+1][y][z].id == 0) createMeshFace(RIGHT,  &chunk, (vec3){x, y, z}); // RIGHT
                    if (chunk.block[x][y+1][z].id == 0) createMeshFace(TOP,    &chunk, (vec3){x, y, z}); // TOP
                    if (chunk.block[x][y][z+1].id == 0) createMeshFace(FRONT,  &chunk, (vec3){x, y, z}); // FRONT
                }
            }
        }
    }


    unsigned int blockTexture;
    unsigned int VBO, VAO;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(chunk.meshData), chunk.meshData, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glGenTextures(1, &blockTexture);
    glBindTexture(GL_TEXTURE_2D, blockTexture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    int width, height, nrChannels;
    unsigned char *data = stbi_load("res/textures/grass.png", &width, &height, &nrChannels, 0);

    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        fprintf(stderr, "Failed to load block texture!\n");
    }

    stbi_image_free(data);

    while (!glfwWindowShouldClose(window.self)) {
        updateWindow();

        useShader(mainShader);
        useCamera(mainShader);

        glBindVertexArray(VAO);

        /*for (int x = 0; x < CHUNK_SIZE_X; x++) {
            for (int y = 0; y < CHUNK_SIZE_Y; y++) {
                for (int z = 0; z < CHUNK_SIZE_Z; z++) {
                    if (chunk.block[x][y][z].id == 1) {
                        glm_mat4_identity(camera.model);
                        glm_translate(camera.model, (vec3){x, y, z});
                        setShaderMat4(mainShader, "model", camera.model);
                        glBindTexture(GL_TEXTURE_2D, blockTexture);
                        glDrawArrays(GL_TRIANGLES, 0, 36);
                    }
                }
            }
        }*/

        glm_mat4_identity(camera.model);
        setShaderMat4(mainShader, "model", camera.model);
        glBindTexture(GL_TEXTURE_2D, blockTexture);
        glDrawArrays(GL_TRIANGLES, 0, chunk.meshSize);

        glfwSwapBuffers(window.self);
        glfwPollEvents();
    }

    terminateWindow();
   
    return 0;
}
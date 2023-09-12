#include "stb/stb_image.h"

#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <string.h>

#include "chunk.h"

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

static void createMeshFace(Direction dir, struct Chunk *chunk, vec3 pos) {
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
            ERROR("Malformed face enumeration while creating chunk mesh!\n");
            exit(EXIT_FAILURE);
            
    }

    for (int i = 0; i < 6; i++) {
        if (chunk->meshSize > CHUNK_MEMORY_BUFFER) {
            ERROR("Chunk memory buffer overflow while creating chunk mesh!\n");
            exit(EXIT_FAILURE);
        }

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

void initChunk(struct Chunk *chunk, vec3 offset) {
    chunk->meshSize = 0;
    chunk->offset[0] = offset[0];
    chunk->offset[1] = offset[1];
    chunk->offset[2] = offset[2];

    memset(chunk->meshData, 0, sizeof(chunk->meshData));
    memset(chunk->block, 0, sizeof(chunk->block));

    // Worldgen
    for (int x = 0; x < CHUNK_SIZE_X; x++) {
        for (int y = 0; y < CHUNK_SIZE_Y; y++) {
            for (int z = 0; z < CHUNK_SIZE_Z; z++) {
                if (y < CHUNK_SIZE_Y-32) chunk->block[x][y][z].id = 1;
            }
        }
    }
}

void constructChunkMesh(struct Chunk *chunk) {
    // Loop through chunks, if block is NOT neighboring, check which side is not neighboring, and add mesh data for that cube face to array
    for (int x = 0; x < CHUNK_SIZE_X; x++) {
        for (int y = 0; y < CHUNK_SIZE_Y; y++) {
            for (int z = 0; z < CHUNK_SIZE_Z; z++) {
                if (chunk->block[x][y][z].id != 0) {
                    if (x-1 < 0) createMeshFace(LEFT,   chunk, (vec3){x, y, z}); // LEFT
                    else if (chunk->block[x-1][y][z].id == 0) createMeshFace(LEFT,   chunk, (vec3){x, y, z}); // LEFT

                    if (y-1 < 0) createMeshFace(BOTTOM, chunk, (vec3){x, y, z}); // BOTTOM
                    else if (chunk->block[x][y-1][z].id == 0) createMeshFace(BOTTOM, chunk, (vec3){x, y, z}); // BOTTOM

                    if (z-1 < 0) createMeshFace(BACK,   chunk, (vec3){x, y, z}); // BACK
                    else if (chunk->block[x][y][z-1].id == 0) createMeshFace(BACK,   chunk, (vec3){x, y, z}); // BACK

                    if (x+1 > CHUNK_SIZE_X) createMeshFace(RIGHT,  chunk, (vec3){x, y, z}); // RIGHT
                    else if (chunk->block[x+1][y][z].id == 0) createMeshFace(RIGHT,  chunk, (vec3){x, y, z}); // RIGHT

                    if (y+1 > CHUNK_SIZE_Y) createMeshFace(TOP,    chunk, (vec3){x, y, z}); // TOP
                    else if (chunk->block[x][y+1][z].id == 0) createMeshFace(TOP,    chunk, (vec3){x, y, z}); // TOP

                    if (z+1 > CHUNK_SIZE_Z) createMeshFace(FRONT,  chunk, (vec3){x, y, z}); // FRONT
                    else if (chunk->block[x][y][z+1].id == 0) createMeshFace(FRONT,  chunk, (vec3){x, y, z}); // FRONT
                }
            }
        }
    }
}

void loadChunk(struct Chunk *chunk) {
    glGenVertexArrays(1, &chunk->VAO);
    glGenBuffers(1, &chunk->VBO);

    glBindVertexArray(chunk->VAO);

    glBindBuffer(GL_ARRAY_BUFFER, chunk->VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(chunk->meshData), chunk->meshData, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glGenTextures(1, &chunk->blockTexture);
    glBindTexture(GL_TEXTURE_2D, chunk->blockTexture);

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
}

void renderChunk(struct Chunk *chunk, struct Shader shader) {
    glBindVertexArray(chunk->VAO);


    glm_mat4_identity(camera.model);
    glm_translate(camera.model, (vec3){chunk->offset[0], chunk->offset[1], chunk->offset[2]});
    setShaderMat4(shader, "model", camera.model);
    glBindTexture(GL_TEXTURE_2D, chunk->blockTexture);
    glDrawArrays(GL_TRIANGLES, 0, chunk->meshSize);
}

void destroyChunk(struct Chunk *chunk) {
    glDeleteVertexArrays(1, &chunk->VAO);
    glDeleteBuffers(1, &chunk->VBO);

    free(chunk);
}
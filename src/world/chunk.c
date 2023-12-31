#include "stb/stb_image.h"

#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <string.h>

#include "worldgen.h"

#include "chunk.h"
#include "block.h"
#include "../game/serialize.h"

const uint32_t cubeVertices[] = {
//  x  y  z  u  v  i  n
    0, 0, 0, 0, 0, 0, 0,// BACK
    1, 0, 0, 1, 0, 0, 0,
    1, 1, 0, 1, 1, 0, 0,
    1, 1, 0, 1, 1, 0, 0,
    0, 1, 0, 0, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 0,

    0, 0, 1, 0, 0, 0, 1, // FRONT
    1, 0, 1, 1, 0, 0, 1,
    1, 1, 1, 1, 1, 0, 1,
    1, 1, 1, 1, 1, 0, 1,
    0, 1, 1, 0, 1, 0, 1,
    0, 0, 1, 0, 0, 0, 1,

    0, 1, 1, 1, 1, 0, 2, // LEFT
    0, 1, 0, 0, 1, 0, 2,
    0, 0, 0, 0, 0, 0, 2,
    0, 0, 0, 0, 0, 0, 2,
    0, 0, 1, 1, 0, 0, 2,
    0, 1, 1, 1, 1, 0, 2,

    1, 1, 1, 1, 1, 0, 3, // RIGHT
    1, 1, 0, 0, 1, 0, 3,
    1, 0, 0, 0, 0, 0, 3,
    1, 0, 0, 0, 0, 0, 3,
    1, 0, 1, 1, 0, 0, 3,
    1, 1, 1, 1, 1, 0, 3,

    0, 0, 0, 0, 1, 0, 4, // BOTTOM
    1, 0, 0, 1, 1, 0, 4,
    1, 0, 1, 1, 0, 0, 4,
    1, 0, 1, 1, 0, 0, 4,
    0, 0, 1, 0, 0, 0, 4,
    0, 0, 0, 0, 1, 0, 4,

    0, 1, 0, 0, 1, 0, 5, // TOP
    1, 1, 0, 1, 1, 0, 5, 
    1, 1, 1, 1, 0, 0, 5,
    1, 1, 1, 1, 0, 0, 5,
    0, 1, 1, 0, 0, 0, 5,
    0, 1, 0, 0, 1, 0, 5
};

static void createMeshFace(Direction dir, struct Chunk *chunk, ivec3 pos, int blockID) {
    int index = 0;

    switch(dir) {
        case BACK:
            index = 0;
            break;
        case FRONT:
            index = 42;
            break;
        case LEFT:
            index = 84;
            break;
        case RIGHT:
            index = 126;
            break;
        case BOTTOM:
            index = 168;
            break;
        case TOP:
            index = 210;
            break;
        default:
            ERROR("Malformed face enumeration while creating chunk mesh!\n");
            exit(EXIT_FAILURE);
            
    }

    if (chunk->meshSize > CHUNK_MEMORY_BUFFER) {
        ERROR("Chunk memory buffer overflow while creating chunk mesh!\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < 6; i++) {
        uint32_t vertex = (cubeVertices[(index + i*7)] + pos[0]) |
                          ((cubeVertices[(index + i*7) + 1] + pos[1]) << 5) |
                          ((cubeVertices[(index + i*7) + 2] + pos[2]) << 14) |
                          (cubeVertices[(index + i*7) + 3] << 19) |
                          (cubeVertices[(index + i*7) + 4] << 20) |
                          ((cubeVertices[(index + i*7) + 5] + getBlockTextureIndex(blockID, dir)) << 21) | 
                          (cubeVertices[(index + i*7) + 6] << 29);

        chunk->meshData[chunk->meshSize++] = vertex;
    }
}


int blockIndex(int x, int y, int z) {
   return (x + z*CHUNK_SIZE_X + y*CHUNK_SIZE_X*CHUNK_SIZE_Z);
}

void initChunk(struct Chunk *chunk, ivec2 position) {
    chunk->isNull = false;
    chunk->meshSize = 0;
    chunk->offset[0] = position[0] * CHUNK_SIZE_X;
    chunk->offset[1] = position[1] * CHUNK_SIZE_Z;

    chunk->worldPos[0] = position[0];
    chunk->worldPos[1] = position[1];

    chunk->meshData = calloc(CHUNK_MEMORY_BUFFER, sizeof(float));
    chunk->blocks = calloc(CHUNK_AREA, sizeof(struct Block));

    /*unsigned char *blockIDs = calloc(CHUNK_AREA, 1);
    for (int i = 0; i < CHUNK_AREA; i++) {
        blockIDs[i] = chunk->blocks[i].id;
    }
    serializeChunk((ivec2){offset[0], offset[1]}, blockIDs);
    free(blockIDs);*/
}

void genChunk(struct Chunk *chunk) {
        // Worldgen
    for (int z = 0; z < CHUNK_SIZE_Z; z++) {
        for (int x = 0; x < CHUNK_SIZE_X; x++) {
            int maxY = noiseHeight((ivec2){x, z}, (ivec2){chunk->offset[0], -chunk->offset[1]});

            for (int y = 0; y < maxY; y++) {
                if (y == maxY - 1) {
                    chunk->blocks[blockIndex(x, y, z)].id = 1;
                } else if (y < maxY - 1 && y > maxY - 6){
                    chunk->blocks[blockIndex(x, y, z)].id = 2;
                } else {
                    chunk->blocks[blockIndex(x, y, z)].id = 3;   
                }
            }
        }
    }
}

void meshChunk(struct Chunk *chunk, struct Chunk *chunkNeighbors) {
    // Loop through chunks, if block is NOT neighboring, check which side is not neighboring, and add mesh data for that cube face to array
    for (int x = 0; x < CHUNK_SIZE_X; x++) {
        for (int y = 0; y < CHUNK_SIZE_Y; y++) {
            for (int z = 0; z < CHUNK_SIZE_Z; z++) {
                if (chunk->blocks[blockIndex(x, y, z)].id != 0) {
                    
                    if (!chunkNeighbors[RIGHT].isNull) {
                        if ((x + 1 < CHUNK_SIZE_X && chunk->blocks[blockIndex(x + 1, y, z)].id == 0) || (x + 1 == CHUNK_SIZE_X && chunkNeighbors[RIGHT].blocks[blockIndex(0, y, z)].id == 0)) createMeshFace(RIGHT,  chunk, (ivec3){x, y, z}, chunk->blocks[blockIndex(x, y, z)].id);
                    }

                    if (!chunkNeighbors[LEFT].isNull) { 
                        if ((x > 0 && chunk->blocks[blockIndex(x - 1, y, z)].id == 0) || (x == 0 && chunkNeighbors[LEFT].blocks[blockIndex(CHUNK_SIZE_X - 1, y, z)].id == 0)) createMeshFace(LEFT,  chunk, (ivec3){x, y, z}, chunk->blocks[blockIndex(x, y, z)].id);
                    }

                    if (!chunkNeighbors[FRONT].isNull) {
                        if ((z + 1 < CHUNK_SIZE_Z && chunk->blocks[blockIndex(x, y, z + 1)].id == 0) || (z + 1 == CHUNK_SIZE_Z && chunkNeighbors[FRONT].blocks[blockIndex(x, y, 0)].id == 0)) createMeshFace(FRONT,  chunk, (ivec3){x, y, z}, chunk->blocks[blockIndex(x, y, z)].id); // FRONT
                    }

                    if (!chunkNeighbors[BACK].isNull) {
                        if ((z > 0 && chunk->blocks[blockIndex(x, y, z - 1)].id == 0) || (z == 0 && chunkNeighbors[BACK].blocks[blockIndex(x, y, CHUNK_SIZE_Z - 1)].id == 0)) createMeshFace(BACK,   chunk, (ivec3){x, y, z}, chunk->blocks[blockIndex(x, y, z)].id); // BACK
                    }
                    
                    if (y-1 < 0); //createMeshFace(BOTTOM, chunk, (ivec3){x, y, z}, chunk->blocks[blockIndex(x, y, z)].id); // BOTTOM
                    else if (chunk->blocks[blockIndex(x, y - 1, z)].id == 0) createMeshFace(BOTTOM, chunk, (ivec3){x, y, z}, chunk->blocks[blockIndex(x, y, z)].id); // BOTTOM

                    if (y+1 >= CHUNK_SIZE_Y) createMeshFace(TOP, chunk, (ivec3){x, y, z}, chunk->blocks[blockIndex(x, y, z)].id); // TOP
                    else if (chunk->blocks[blockIndex(x, y + 1, z)].id == 0) createMeshFace(TOP, chunk, (ivec3){x, y, z}, chunk->blocks[blockIndex(x, y, z)].id); // TOP
                }
            }
        }
    }
}

void bindChunk(struct Chunk *chunk) {
    glGenVertexArrays(1, &chunk->VAO);
    glGenBuffers(1, &chunk->VBO);

    glBindVertexArray(chunk->VAO);

    glBindBuffer(GL_ARRAY_BUFFER, chunk->VBO);
    glBufferData(GL_ARRAY_BUFFER, chunk->meshSize * sizeof(uint32_t), chunk->meshData, GL_STATIC_DRAW);

    glVertexAttribIPointer(0, 1, GL_UNSIGNED_INT, sizeof(uint32_t), (void*)0);
    glEnableVertexAttribArray(0);
}

void renderChunk(struct Chunk *chunk, struct Shader shader) {
    glBindVertexArray(chunk->VAO);
    glActiveTexture(GL_TEXTURE0); // Use texture unit 0
    glBindTexture(GL_TEXTURE_2D_ARRAY, getArrayTexture());
    setShaderInt(shader, "arrayTexture", 0);

    glm_mat4_identity(camera.model);
    glm_translate(camera.model, (vec3){chunk->offset[0], 0, chunk->offset[1]});
    setShaderMat4(shader, "model", camera.model);

    glDrawArrays(GL_TRIANGLES, 0, chunk->meshSize);
}

void destroyChunk(struct Chunk *chunk) {
    glDeleteVertexArrays(1, &chunk->VAO);
    glDeleteBuffers(1, &chunk->VBO);

    free(chunk->meshData);
    free(chunk->blocks);
    free(chunk);
}
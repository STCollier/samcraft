#include "stb/stb_image.h"

#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <string.h>

#include "worldgen.h"

#include "chunk.h"
#include "block.h"
#include "../game/serialize.h"

#define RAND(min, max) (rand() % (max + 1 - min) + min)

const int cubeVertices[] = {
//  x  y  z  u  v  i
    0, 0, 0, 0, 0, 0, // BACK
    1, 0, 0, 1, 0, 0,
    1, 1, 0, 1, 1, 0,
    1, 1, 0, 1, 1, 0,
    0, 1, 0, 0, 1, 0,
    0, 0, 0, 0, 0, 0,

    0, 0, 1, 0, 0, 0, // FRONT
    1, 0, 1, 1, 0, 0,
    1, 1, 1, 1, 1, 0,
    1, 1, 1, 1, 1, 0,
    0, 1, 1, 0, 1, 0,
    0, 0, 1, 0, 0, 0,

    0, 1, 1, 0, 0, 0, // LEFT
    0, 1, 0, -1, 0, 0,
    0, 0, 0, -1, -1, 0,
    0, 0, 0, -1, -1, 0,
    0, 0, 1, 0, -1, 0,
    0, 1, 1, 0, 0, 0,

    1, 1, 1, 0, 0, 0, // RIGHT
    1, 1, 0, -1, 0, 0,
    1, 0, 0, -1, -1, 0,
    1, 0, 0, -1, -1, 0,
    1, 0, 1, 0, -1, 0,
    1, 1, 1, 0, 0, 0,

    0, 0, 0, 0, 1, 0, // BOTTOM
    1, 0, 0, 1, 1, 0,
    1, 0, 1, 1, 0, 0,
    1, 0, 1, 1, 0, 0,
    0, 0, 1, 0, 0, 0,
    0, 0, 0, 0, 1, 0,

    0, 1, 0, 0, 1, 0, // TOP
    1, 1, 0, 1, 1, 0, 
    1, 1, 1, 1, 0, 0,
    1, 1, 1, 1, 0, 0,
    0, 1, 1, 0, 0, 0,
    0, 1, 0, 0, 1, 0
};

static void createMeshFace(Direction dir, struct Chunk *chunk, vec3 pos, int blockID) {
    int index = 0;

    switch(dir) {
        case BACK:
            index = 0;
            break;
        case FRONT:
            index = 36;
            break;
        case LEFT:
            index = 72;
            break;
        case RIGHT:
            index = 108;
            break;
        case BOTTOM:
            index = 144;
            break;
        case TOP:
            index = 180;
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
        chunk->meshData[chunk->meshSize] = cubeVertices[(index + i*6)] + pos[0];
        chunk->meshData[chunk->meshSize + 1] = cubeVertices[(index + i*6) + 1] + pos[1];
        chunk->meshData[chunk->meshSize + 2] = cubeVertices[(index + i*6) + 2] + pos[2];

        // UVs
        chunk->meshData[chunk->meshSize + 3] = cubeVertices[(index + i*6) + 3];
        chunk->meshData[chunk->meshSize + 4] = cubeVertices[(index + i*6) + 4];

        if (blockID != 0) {
            chunk->meshData[chunk->meshSize + 5] = cubeVertices[(index + i*6) + 5] + getBlockTextureIndex(blockID, dir);
        }

        chunk->meshSize += 6;
    }
}


int blockIndex(int x, int y, int z) {
   return (x + z*CHUNK_SIZE_X + y*CHUNK_SIZE_X*CHUNK_SIZE_Z);
}

void initChunk(struct Chunk *chunk, ivec2 offset) {
    chunk->isNull = false;
    chunk->meshSize = 0;
    chunk->offset[0] = offset[0];
    chunk->offset[1] = offset[1];

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
                if (y > randInRange(100, 110)) chunk->blocks[blockIndex(x, y, z)].id = 5;
                else if (y > randInRange(80, 100)) chunk->blocks[blockIndex(x, y, z)].id = 3;
                else if (y > randInRange(60, 80)) chunk->blocks[blockIndex(x, y, z)].id = 2;
                else chunk->blocks[blockIndex(x, y, z)].id = 1;
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
                        if ((x + 1 < CHUNK_SIZE_X && chunk->blocks[blockIndex(x + 1, y, z)].id == 0) || (x + 1 == CHUNK_SIZE_X && chunkNeighbors[RIGHT].blocks[blockIndex(0, y, z)].id == 0)) createMeshFace(RIGHT,  chunk, (vec3){x, y, z}, chunk->blocks[blockIndex(x, y, z)].id);
                    }

                    if (!chunkNeighbors[LEFT].isNull) { 
                        if ((x > 0 && chunk->blocks[blockIndex(x - 1, y, z)].id == 0) || (x == 0 && chunkNeighbors[LEFT].blocks[blockIndex(CHUNK_SIZE_X - 1, y, z)].id == 0)) createMeshFace(LEFT,  chunk, (vec3){x, y, z}, chunk->blocks[blockIndex(x, y, z)].id);
                    }

                    if (!chunkNeighbors[FRONT].isNull) {
                        if ((z + 1 < CHUNK_SIZE_Z && chunk->blocks[blockIndex(x, y, z + 1)].id == 0) || (z + 1 == CHUNK_SIZE_Z && chunkNeighbors[FRONT].blocks[blockIndex(x, y, 0)].id == 0)) createMeshFace(FRONT,  chunk, (vec3){x, y, z}, chunk->blocks[blockIndex(x, y, z)].id); // FRONT
                    }

                    if (!chunkNeighbors[BACK].isNull) {
                        if ((z > 0 && chunk->blocks[blockIndex(x, y, z - 1)].id == 0) || (z == 0 && chunkNeighbors[BACK].blocks[blockIndex(x, y, CHUNK_SIZE_Z - 1)].id == 0)) createMeshFace(BACK,   chunk, (vec3){x, y, z}, chunk->blocks[blockIndex(x, y, z)].id); // BACK
                    }
                    
                    if (y-1 < 0); //createMeshFace(BOTTOM, chunk, (vec3){x, y, z}, chunk->blocks[blockIndex(x, y, z)].id); // BOTTOM
                    else if (chunk->blocks[blockIndex(x, y - 1, z)].id == 0) createMeshFace(BOTTOM, chunk, (vec3){x, y, z}, chunk->blocks[blockIndex(x, y, z)].id); // BOTTOM

                    if (y+1 >= CHUNK_SIZE_Y) createMeshFace(TOP, chunk, (vec3){x, y, z}, chunk->blocks[blockIndex(x, y, z)].id); // TOP
                    else if (chunk->blocks[blockIndex(x, y + 1, z)].id == 0) createMeshFace(TOP, chunk, (vec3){x, y, z}, chunk->blocks[blockIndex(x, y, z)].id); // TOP
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
    glBufferData(GL_ARRAY_BUFFER, chunk->meshSize * sizeof(int), chunk->meshData, GL_STATIC_DRAW);

    // Position (x, y, z) attribute
    glVertexAttribIPointer(0, 3, GL_INT, 6 * sizeof(int), (void*)0);
    glEnableVertexAttribArray(0);

    // Texture coords (u, v) attribute
    glVertexAttribIPointer(1, 2, GL_INT, 6 * sizeof(int), (void*)(3 * sizeof(int)));
    glEnableVertexAttribArray(1);

    // Texture index (i) attribute
    glVertexAttribIPointer(2, 1, GL_INT, 6 * sizeof(int), (void*)(5 * sizeof(int)));
    glEnableVertexAttribArray(2);
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
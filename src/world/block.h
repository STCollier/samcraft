#ifndef BLOCK_H
#define BLOCK_H

#include "../engine/util/common.h"
#include "../engine/core/shader.h"

// These are the only two preset block IDs, the rest are customizable through lua
#define BLOCK_AIR 0
#define BLOCK_NULL 1

struct Block {
    uint8_t id; // 1 byte values, which can hold 256 unique block types/states
};

struct MaterialIndices {
    int diffuseIndex, normalIndex;
    int reserved01, reserved02;
};

struct Textures {
    const char* diffuse[6];
    const char* normal[6];
    size_t diffuseIndex, normalIndex;
};

struct BlockData {
    int id;
    const char* name;
    struct Textures textures;
    struct MaterialIndices materialIndices[6];
    const char* normal;
    float hardness;
};

void blockdata_loadLuaData();
void blockdata_loadMaterials(shader_t shader);

unsigned int block_getDiffuseArrayTexture();
unsigned int block_getNormalArrayTexture();
unsigned int block_getBreakArrayTexture();
int block_getID(const char* name);
int block_getDiffuseIndex(int ID, Direction dir);
int block_getNormalIndex(int ID, Direction dir);
float block_getHardnessValue(int ID);

#endif
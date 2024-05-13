#ifndef BLOCK_H
#define BLOCK_H

#include "../engine/util/common.h"

// These are the only two preset block IDs, the rest are customizable through lua
#define BLOCK_AIR 0
#define BLOCK_NULL 1

struct Block {
    uint8_t id; // 1 byte values, which can hold 256 unique block types/states
};

struct BlockData {
    int id;
    const char* name;
    const char* textures[6];
    float hardness;
};

void blockdata_loadLuaData();
void blockdata_loadArrayTexture();

unsigned int block_getArrayTexture();
unsigned int block_getBreakArrayTexture();
int block_getID(const char* name);
int block_getTextureIndex(int id, Direction dir);
float block_getHardnessValue(int blockID);


extern struct BlockData blockData[256];

#endif
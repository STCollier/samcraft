#ifndef BLOCK_H
#define BLOCK_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <lua/lua.h>
#include <lua/lauxlib.h>
#include <lua/lualib.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#include "../engine/util.h"
#include "../engine/types.h"

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
};

void blockdata_loadLuaData();
void blockdata_loadArrayTexture();

unsigned int block_getArrayTexture();
int block_getID(const char* name);
int block_getTextureIndex(int id, Direction dir);


extern struct BlockData blockData[256];

#endif
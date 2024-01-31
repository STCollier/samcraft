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

enum Direction {
    RIGHT,
    LEFT,
    FRONT,
    BACK,
    TOP,
    BOTTOM
};

struct Block {
    unsigned char id; // 1 byte values, which can hold 256 unique block types/states
};

struct BlockData {
    int id;
    const char* name;
    char* textures[6];
};

void block_loadLuaData();
void block_loadArrayTexture();
unsigned int block_getArrayTexture();
int block_getTextureIndex(int blockID, enum Direction dir);

extern struct BlockData blockData[256];

#endif
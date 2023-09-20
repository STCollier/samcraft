#ifndef BLOCK_H
#define BLOCK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "../game/util.h"

typedef enum {
    RIGHT,
    LEFT,
    FRONT,
    BACK,
    TOP,
    BOTTOM
} Direction;

struct Block {
    unsigned char id; // 1 byte values, which can hold 256 unique block types/states
};

struct BlockData {
    int id;
    const char* name;
    char* textures[6];
};

void loadBlocksFromFile();
void loadArrayTexture();
unsigned int getArrayTexture();

extern struct BlockData blockData[256];

#endif
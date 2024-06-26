#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <lua/lua.h>
#include <lua/lauxlib.h>
#include <lua/lualib.h>
#include <dirent.h>
#include "stb/stb_image.h"

#include "../engine/util/util.h"
#include "../engine/util/types.h"

#include "block.h"

struct BlockData blockData[256];

const char* faces[6] = {"right", "left", "front", "back", "top", "bottom"};
char texturePaths[256*6][300] = {0};
char textureArray[256*6][300] = {0};
unsigned int arrayTexture = 0;
unsigned int blockBreakArrayTexture = 0;
int numTextures = 0;
int blockID = 2; // 0 and 1 are taken up by BLOCK_AIR and BLOCK_NULL, the rest are free

static void loadLuaFile(const char* filename) {
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);

    const char* blockTextures[6] = {0};

    if (luaL_dofile(L, filename)) {
        ERROR_MSG("Error loading Lua block file:", lua_tostring(L, -1));
        lua_close(L);
        exit(EXIT_FAILURE);
    } else {
        LOG_MSG("Loaded block file", filename);
    }

    lua_getGlobal(L, "block", "The field 'block' is not a table at", filename);
    const char* blockName = lua_getString(L, "name", "The field 'name' is invalid at", filename);
    float blockHardness = lua_getFloat(L, "hardness", "The field 'hardness' is invalid at", filename);
    lua_getField(L, "textures", "The field 'textures' is not a table at", filename);

    lua_getfield(L, -1, "all");
    if (!lua_isnil(L, -1)) {
        for (int i = 0; i < 6; i++) {
            blockTextures[i] = lua_tostring(L, -1);
        }
        lua_pop(L, 1);
    } else {
        lua_pop(L, 1);
        for (int i = 0; i < 6; i++) {
            const char* textures = lua_getString(L, faces[i], "Malformed texture specifier at", filename);
            blockTextures[i] = textures;
        }
    }

    if (blockData[blockID].id == -1) {
        blockData[blockID].id = blockID;
        blockData[blockID].name = blockName;
        blockData[blockID].hardness = blockHardness;
        for (int i = 0; i < 6; i++ ) {
            blockData[blockID].textures[i] = blockTextures[i];
        }

        blockID++;
    } else {
        // Hopefully this never happens
        ERROR_IMSG("Attempting to overwrite previous block data at ID", blockID);
        exit(EXIT_FAILURE);
    }
}

void blockdata_loadLuaData() {
    // Set all block data to be -1 (no data)
    for (int i = 0; i < 256; i++) {
        blockData[i].id = -1;
    }

    DIR *d;
    struct dirent *dir;
    d = opendir("./client/blocks/");

    if (d) {
        while ((dir = readdir(d)) != NULL) {
            if (dir->d_type == DT_DIR || strcmp(strrchr(dir->d_name, '.') + 1, "lua") != 0) continue;
            
            char filename[64] = "./client/blocks/";
            strcat(filename, dir->d_name);

            loadLuaFile(filename);
        }
        closedir(d);
    }

    blockData[BLOCK_NULL].id = BLOCK_NULL;
    blockData[BLOCK_NULL].name = "null";
    blockData[BLOCK_NULL].hardness = 1.0;
    for (int i = 0; i < 6; i++) {
        blockData[BLOCK_NULL].textures[i] = "null.png";
    }
}


void blockdata_loadArrayTexture() {
    glGenTextures(1, &arrayTexture);
    glBindTexture(GL_TEXTURE_2D_ARRAY, arrayTexture);

    DIR *d;
    struct dirent *dir;
    d = opendir("./res/textures");
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            if (dir->d_type == DT_DIR || strcmp(strrchr(dir->d_name, '.') + 1, "png") != 0) continue;
    
            snprintf(textureArray[numTextures], sizeof(textureArray[numTextures]), "%s", dir->d_name);
            snprintf(texturePaths[numTextures], sizeof(texturePaths[numTextures]), "res/textures/%s", dir->d_name);
            numTextures++;
        }
    }
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);    
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Allocate texture storage with GL_RGBA8 internal format
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_SRGB8_ALPHA8, 16, 16, numTextures, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    int width, height, channels;
    for (int i = 0; i < numTextures; i++) {
        unsigned char* texture = stbi_load(texturePaths[i], &width, &height, &channels, 4);

        if (!texture) {
            ERROR_MSG("Failed to load texture at path", texturePaths[i]);
            exit(EXIT_FAILURE);
        } else {
            LOG_MSG("Loaded block texture", texturePaths[i]);
        }

        // Upload each image as a 16x16x1 slice of the array
        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, 16, 16, 1, GL_RGBA, GL_UNSIGNED_BYTE, texture);
        stbi_image_free(texture);
    }

    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

    /*--------------*/

    glGenTextures(1, &blockBreakArrayTexture);
    glBindTexture(GL_TEXTURE_2D_ARRAY, blockBreakArrayTexture);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);    
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Allocate texture storage with GL_RGBA8 internal format
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_SRGB8_ALPHA8, 16, 16, 5, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    int bwidth, bheight, bchannels;
    char texSrc[64];
    for (int i = 0; i < 5; i++) {
        snprintf(texSrc, 64, "res/textures/break%d.png", i+1);
        unsigned char* texture = stbi_load(texSrc, &bwidth, &bheight, &bchannels, 4);

        if (!texture) {
            ERROR_MSG("Failed to load texture at path", texSrc);
            exit(EXIT_FAILURE);
        } else {
            LOG_MSG("Loaded block texture", texSrc);
        }

        // Upload each image as a 16x16x1 slice of the array
        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, 16, 16, 1, GL_RGBA, GL_UNSIGNED_BYTE, texture);
        stbi_image_free(texture);
    }

    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
}

unsigned int block_getArrayTexture() {
    return arrayTexture;
}

unsigned int block_getBreakArrayTexture() {
    return blockBreakArrayTexture;
}

static bool noBlockErr[256];
int block_getID(const char* name) {
    // Starting at i = 2 because block ID slots 0 and 1 are taken up by BLOCK_AIR and BLOCK_NULL
    for (int i = 2; i < blockID; i++) {
        struct BlockData block = blockData[i];

        if (strcmp(block.name, name) == 0) {
            return i;
        }
    }

    if (!noBlockErr[hash8(name)]) {
        WARN_MSG2("Failed to find block under name", name, "Using null block replacement");
        noBlockErr[hash8(name)] = true;
    }

    return BLOCK_NULL;
}

static bool noTextureErr[256];
int block_getTextureIndex(int blockID, Direction dir) {
    for (int i = 0; i < numTextures; i++) {
        if (strcmp(blockData[blockID].textures[dir], textureArray[i]) == 0) {
            return i;
        }
    }
    
    if (!noTextureErr[blockID]) {
        fprintf(stdout, "\x1B[0m%s:%d: \x1B[0;35m[WARNING]\x1B[0m Failed to locate %s texture in %s.lua (%s). Using null texture as replacement\n", __FILE__, __LINE__, faces[blockID], blockData[blockID].name, blockData[blockID].textures[dir]);
        noTextureErr[blockID] = true;
    }

    return block_getTextureIndex(BLOCK_NULL, 0); // Setting to 0 because it doesn't matter; can be any arbitrary value due to all null textures being the same
}

float block_getHardnessValue(int blockID) {
    return blockData[blockID].hardness;
}
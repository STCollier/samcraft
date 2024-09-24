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

static struct BlockData blockData[256];

static const char* faces[6] = {"top", "bottom", "right", "left", "front", "back"};

static char diffuseNames[256*6][256];
static char normalNames[256*6][256];

static unsigned int diffuseArray, normalArray, blockBreakArray;
static int diffuseCount = 0, normalCount = 0;

static int blockID = 2; // 0 and 1 are taken up by BLOCK_AIR and BLOCK_NULL, the rest are free

static void loadLuaFile(const char* filename) {
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);

    const char* blockTextures[6] = {0};
    const char* blockNormals[6] = {0};

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
    int blockLight = lua_getInt(L, "light", "The field 'light' is invalid at", filename);

    lua_getField(L, "textures", "The field 'textures' is not a table at", filename);
    lua_getField(L, "diffuse", "The field 'diffuse' is not a table at", filename);
    
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

    lua_pop(L, 1);

    lua_getField(L, "normal", "The field 'normal' is not a table at", filename);
    
    lua_getfield(L, -1, "all");
    if (!lua_isnil(L, -1)) {
        for (int i = 0; i < 6; i++) {
            blockNormals[i] = lua_tostring(L, -1);
        }
        lua_pop(L, 1);
    } else {
        lua_pop(L, 1);
        for (int i = 0; i < 6; i++) {
            const char* textures = lua_getString(L, faces[i], "Malformed normal specifier at", filename);
            blockNormals[i] = textures;
        }
    }

    if (blockData[blockID].id == -1) {
        blockData[blockID].id = blockID;
        blockData[blockID].name = blockName;
        blockData[blockID].hardness = blockHardness;
        blockData[blockID].light = blockLight;
        for (int i = 0; i < 6; i++ ) {
            blockData[blockID].textures.diffuse[i] = blockTextures[i];
            blockData[blockID].textures.normal[i] = blockNormals[i];
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
    blockData[BLOCK_NULL].light = 0;
    for (int i = 0; i < 6; i++) {
        blockData[BLOCK_NULL].textures.diffuse[i] = "null.png";
        blockData[BLOCK_NULL].textures.normal[i] = NULL;
    }
}


void blockdata_loadMaterials(shader_t shader) {
    char diffusePaths[256*6][256];
    char normalPaths[256*6][256];

    glGenTextures(1, &diffuseArray);
    glBindTexture(GL_TEXTURE_2D_ARRAY, diffuseArray);

    DIR *d;
    struct dirent *dir;
    d = opendir("./res/textures/block/diffuse");
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            if (dir->d_type == DT_DIR || strcmp(strrchr(dir->d_name, '.') + 1, "png") != 0) continue;
    
            snprintf(diffuseNames[diffuseCount], sizeof(diffuseNames[diffuseCount]), "%s", dir->d_name);
            snprintf(diffusePaths[diffuseCount], sizeof(diffusePaths[diffuseCount]), "res/textures/block/diffuse/%s", dir->d_name);
            diffuseCount++;
        }
    }
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);    
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Allocate texture storage with GL_RGBA8 internal format
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_SRGB8_ALPHA8, 16, 16, diffuseCount, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    int width, height, channels;
    for (int i = 0; i < diffuseCount; i++) {
        unsigned char* texture = stbi_load(diffusePaths[i], &width, &height, &channels, 4);

        if (!texture) {
            ERROR_MSG("Failed to load texture at path", diffusePaths[i]);
            exit(EXIT_FAILURE);
        } else {
            LOG_MSG("Loaded block texture", diffusePaths[i]);
        }

        // Upload each image as a 16x16x1 slice of the array
        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, 16, 16, 1, GL_RGBA, GL_UNSIGNED_BYTE, texture);
        stbi_image_free(texture);
    }

    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

    /*--------------*/

    glGenTextures(1, &normalArray);
    glBindTexture(GL_TEXTURE_2D_ARRAY, normalArray);

    DIR *dn;
    struct dirent *dirn;
    dn = opendir("./res/textures/block/normal");
    if (dn) {
        while ((dirn = readdir(dn)) != NULL) {
            if (dirn->d_type == DT_DIR || strcmp(strrchr(dirn->d_name, '.') + 1, "png") != 0) continue;
    
            snprintf(normalNames[normalCount], sizeof(normalNames[normalCount]), "%s", dirn->d_name);
            snprintf(normalPaths[normalCount], sizeof(normalPaths[normalCount]), "res/textures/block/normal/%s", dirn->d_name);
            normalCount++;
        }
    }
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);    
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Allocate texture storage with GL_RGBA8 internal format
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_SRGB8_ALPHA8, 160, 160, normalCount, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    int nwidth, nheight, nchannels;
    for (int i = 0; i < normalCount; i++) {
        unsigned char* texture = stbi_load(normalPaths[i], &nwidth, &nheight, &nchannels, 4);

        if (!texture) {
            ERROR_MSG("Failed to load texture at path", normalPaths[i]);
            exit(EXIT_FAILURE);
        } else {
            LOG_MSG("Loaded normal texture", normalPaths[i]);
        }

        // Upload each image as a 16x16x1 slice of the array
        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, 160, 160, 1, GL_RGBA, GL_UNSIGNED_BYTE, texture);
        stbi_image_free(texture);
    }

    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);


    /**************/

    glGenTextures(1, &blockBreakArray);
    glBindTexture(GL_TEXTURE_2D_ARRAY, blockBreakArray);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);    
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Allocate texture storage with GL_RGBA8 internal format
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_SRGB8_ALPHA8, 16, 16, 5, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    int bwidth, bheight, bchannels;
    char texSrc[64];
    for (int i = 0; i < 5; i++) {
        snprintf(texSrc, 64, "res/textures/block/diffuse/break%d.png", i+1);
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

    struct MaterialIndices* materialIndices = malloc(sizeof(struct MaterialIndices) * 256 * 6);

    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 6; j++) {
            materialIndices[6 * i + j].diffuseIndex = block_getDiffuseIndex(i, j);
            materialIndices[6 * i + j].normalIndex = block_getNormalIndex(i, j);
        }
    }

    glUniformBlockBinding(shader.ID, glGetUniformBlockIndex(shader.ID, "BlockData"), 0);

    unsigned int blockMaterialUBO;
    glGenBuffers(1, &blockMaterialUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, blockMaterialUBO);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(struct MaterialIndices) * 256 * 6, NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glBindBufferRange(GL_UNIFORM_BUFFER, 0, blockMaterialUBO, 0, sizeof(struct MaterialIndices) * 256 * 6);

    glBindBuffer(GL_UNIFORM_BUFFER, blockMaterialUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(struct MaterialIndices) * 256 * 6, materialIndices);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    free(materialIndices);
}

unsigned int block_getDiffuseArrayTexture() {
    return diffuseArray;
}

unsigned int block_getNormalArrayTexture() {
    return normalArray;
}

unsigned int block_getBreakArrayTexture() {
    return blockBreakArray;
}

int block_getID(const char* name) {
    // Starting at i = 2 because block ID slots 0 and 1 are taken up by BLOCK_AIR and BLOCK_NULL
    for (int i = 2; i < blockID; i++) {
        struct BlockData block = blockData[i];

        if (strcmp(block.name, name) == 0) {
            return i;
        }
    }

    return BLOCK_NULL;
}

int block_getDiffuseIndex(int ID, Direction dir) {
    if (ID == 0 || ID == 1 || ID > blockID - 1) return -1;

    for (int i = 0; i < diffuseCount; i++) {
        if (strcmp(blockData[ID].textures.diffuse[dir], diffuseNames[i]) == 0) {
            return i;
        }
    }

    return block_getDiffuseIndex(BLOCK_NULL, 0);
}

int block_getNormalIndex(int ID, Direction dir) {
    if (ID == 0 || ID == 1 || ID > blockID - 1) return -1;

    for (int i = 0; i < normalCount; i++) {
        if (strcmp(blockData[ID].textures.normal[dir], normalNames[i]) == 0) {
            return i;
        }
    }

    return -1;
}

float block_getHardnessValue(int ID) {
    return blockData[ID].hardness;
}

unsigned int block_getLightLevel(int ID) {
    return blockData[ID].light;
}
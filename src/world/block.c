#include <stb/stb_image.h>
#include "block.h"

struct BlockData blockData[256];

const char* faces[6] = {"right", "left", "front", "back", "top", "bottom"};
char texturePaths[256*6][300] = {0};
char textureArray[256*6][300] = {0};
unsigned int arrayTexture = 0;
int numTextures = 0;
int blockID = 2; // 0 and 1 are taken up by BLOCK_AIR and BLOCK_NULL, the rest are free

static void loadLuaFile(const char* filename) {
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);

    const char* blockTextures[6] = {0};
    const char* blockName = 0;

    if (luaL_dofile(L, filename)) {
        ERROR_MSG("Error loading Lua block file:", lua_tostring(L, -1));
        lua_close(L);
        exit(EXIT_FAILURE);
    } else {
        LOG_MSG("Loaded block file", filename);
    }

    lua_getGlobal(L, "block", "The field 'block' is not a table at", filename);
    blockName = lua_getString(L, "name", "The field 'name' is not a string at", filename);
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
            if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0 || strcmp(strrchr(dir->d_name, '.') + 1, "lua") != 0) continue;
            
            char filename[64] = "./client/blocks/";
            strcat(filename, dir->d_name);

            loadLuaFile(filename);
        }
        closedir(d);
    }

    blockData[BLOCK_NULL].id = BLOCK_NULL;
    blockData[BLOCK_NULL].name = "null";
    for (int i = 0; i < 6; i++) {
        blockData[BLOCK_NULL].textures[i] = "null.png";
    }
}


void blockdata_loadArrayTexture() {
    GL_CHECK(glGenTextures(1, &arrayTexture));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D_ARRAY, arrayTexture));

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);    
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    DIR *d;
    struct dirent *dir;
    d = opendir("./res/textures");
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0 || strcmp(strrchr(dir->d_name, '.') + 1, "png") != 0) continue;
    
            snprintf(textureArray[numTextures], sizeof(textureArray[numTextures]), "%s", dir->d_name);
            snprintf(texturePaths[numTextures], sizeof(texturePaths[numTextures]), "res/textures/%s", dir->d_name);
            numTextures++;
        }
    }

    // Allocate texture storage with GL_RGBA8 internal format
    GL_CHECK(glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_SRGB8_ALPHA8, 16, 16, numTextures, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL));

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
        GL_CHECK(glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, 16, 16, 1, GL_RGBA, GL_UNSIGNED_BYTE, texture));
        stbi_image_free(texture);
    }
}

unsigned int block_getArrayTexture() {
    return arrayTexture;
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
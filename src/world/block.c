#include <stb/stb_image.h>
#include "block.h"

struct BlockData blockData[256];

char texturePaths[256*6][300] = {0};
char textureArray[256*6][300] = {0};
unsigned int arrayTexture = 0;
int numTextures = 0;

void block_loadLuaData() {
    const char* faces[6] = {"right", "left", "front", "back", "top", "bottom"};
    int blockID = 0;
    const char* blockName = "";
    char* blockTextures[6] = {0};

    for (int i = 0; i < 256; i++) {
        blockData[i].id = -1;
    }

    lua_State *L = luaL_newstate();
    luaL_openlibs(L);

    DIR *d;
    struct dirent *dir;
    d = opendir("./src/world/blocks");
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0 || strcmp(strrchr(dir->d_name, '.') + 1, "lua") != 0) continue;
            
            char filename[64] = "./src/world/blocks/";
            strcat(filename, dir->d_name);

            if (luaL_dofile(L, filename)) {
                ERROR_MSG("Error loading Lua block file:", lua_tostring(L, -1));
                lua_close(L);
                exit(EXIT_FAILURE);
            } else {
                LOG_MSG("Loaded block file", dir->d_name);
            }

            lua_getglobal(L, "block");
            if (!lua_istable(L, -1)) {
                ERROR_MSG("Could not find block data under table 'block' at", dir->d_name);
                lua_close(L);
                exit(EXIT_FAILURE);
            }

            lua_getfield(L, -1, "id");
            if (!lua_isnumber(L, -1)) {
                ERROR_MSG("The 'id' field is not a number at", dir->d_name);
                lua_close(L);
                exit(EXIT_FAILURE);
            }
            blockID = lua_tointeger(L, -1);
            lua_pop(L, 1);

            lua_getfield(L, -1, "name");
            if (!lua_isstring(L, -1)) {
                ERROR_MSG("The 'name' field is not a string at", dir->d_name);
                lua_close(L);
                exit(EXIT_FAILURE);
            }
            blockName = lua_tostring(L, -1);
            lua_pop(L, 1);

            lua_getfield(L, -1, "textures");
            if (!lua_istable(L, -1)) {
                ERROR_MSG("The 'textures' field is not a table at", dir->d_name);
                lua_close(L);
                exit(EXIT_FAILURE);
            }

            for (int i = 0; i < 6; i++) {
                lua_getfield(L, -1, faces[i]);
                if (!lua_isstring(L, -1)) {
                    ERROR_MSG("The texture direction is not a string: ", faces[i]);
                    lua_close(L);
                    exit(EXIT_FAILURE);
                }
                blockTextures[i] = strdup(lua_tostring(L, -1));
                lua_pop(L, 1);
            }

            if (blockData[blockID].id == -1) {
                blockData[blockID].id = blockID;
                blockData[blockID].name = blockName;
                for (int i = 0; i < 6; i++ ) {
                    blockData[blockID].textures[i] = blockTextures[i];
                }
            } else {
                ERROR_IMSG("Attempting to overwrite previous block data at ID", blockID);
                exit(EXIT_FAILURE);
            }
        }
        closedir(d);
    }
}


void block_loadArrayTexture() {
    glGenTextures(1, &arrayTexture);
    glBindTexture(GL_TEXTURE_2D_ARRAY, arrayTexture);

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

    int width, height, channels;
    for (int i = 0; i < numTextures; i++) {
        unsigned char* texture = stbi_load(texturePaths[i], &width, &height, &channels, 0);

        if (!texture) {
            WARN_MSG("Failed to load texture at path", texturePaths[i]);
            continue;
        } else {
            LOG_MSG("Loaded block texture", texturePaths[i]);
        }

        glTexImage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA, 16, 16, numTextures, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture);
        stbi_image_free(texture);
    }
}

unsigned int block_getArrayTexture() {
    return arrayTexture;
}

int block_getTextureIndex(int blockID, enum Direction dir) {
    for (int i = 0; i < numTextures; i++) {
        if (strcmp(blockData[blockID].textures[dir], textureArray[i]) != 0) {
            continue;
        } else {
            return i;
        }
    }
    
    /*ERROR_IMSG("Could not locate texture index at index", blockID);
    exit(EXIT_FAILURE);*/
} 
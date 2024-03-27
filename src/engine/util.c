#include "util.h"

void checkOpenGLErr(const char* stmt, const char* fname, int line) {
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        printf("OpenGL error %08x, at %s:%i - for %s\n", err, fname, line, stmt);
        abort();
    }
}

void ivec3s_to_ivec3(ivec3s in, ivec3 out) {
    glm_ivec3_copy((ivec3){
        in.x,
        in.y,
        in.z
    }, out);
}

void ivec2s_to_ivec2(ivec2s in, ivec2 out) {
    glm_ivec3_copy((ivec2){
        in.x,
        in.y,
    }, out);
}

uint8_t hash8(const char* h) {
    uint8_t hash = 0;

    const char* p = h;
    while (*p) {
        hash = (hash * 31) + *p;
        p++;
    }

    return hash;
}

int idist2d(ivec2 a, ivec2 b) {
    return sqrt((b[0] - a[0])*(b[0] - a[0]) + (b[1] - a[1])*(b[1] - a[1]));
}

int idist3d(ivec3 a, ivec3 b) {
    return sqrt((b[0] - a[0])*(b[0] - a[0]) + (b[1] - a[1])*(b[1] - a[1]) + (b[2] - a[2])*(b[2] - a[2]));
}

int randInt(int min, int max) {
   return min + rand() / (RAND_MAX / (max - min + 1) + 1);
}

int lua_getInt(lua_State *L, const char* field, const char* err, const char* msg) {
    lua_getfield(L, -1, field);
    if (!lua_isnumber(L, -1)) {
        ERROR_MSG(err, msg);
        lua_close(L);
        exit(EXIT_FAILURE);
    }
    int i = lua_tointeger(L, -1);
    lua_pop(L, 1);

    return i;
}

float lua_getFloat(lua_State *L, const char* field, const char* err, const char* msg) {
    lua_getfield(L, -1, field);
    if (!lua_isnumber(L, -1)) {
        ERROR_MSG(err, msg);
        lua_close(L);
        exit(EXIT_FAILURE);
    }
    float f = lua_tonumber(L, -1);
    lua_pop(L, 1);

    return f;
}

bool lua_getBool(lua_State *L, const char* field, const char* err, const char* msg) {
    lua_getfield(L, -1, field);
    if (!lua_isboolean(L, -1)) {
        ERROR_MSG(err, msg);
        lua_close(L);
        exit(EXIT_FAILURE);
    }
    bool b = lua_toboolean(L, -1);
    lua_pop(L, 1);

    return b;
}

const char* lua_getString(lua_State *L, const char* field, const char* err, const char* msg) {
    lua_getfield(L, -1, field);
    if (!lua_isstring(L, -1)) {
        ERROR_MSG(err, msg);
        lua_close(L);
        exit(EXIT_FAILURE);
    }
    const char* s = lua_tostring(L, -1);
    lua_pop(L, 1);

    return s;
}

void lua_getGlobal(lua_State *L, const char* field, const char* err, const char* msg) {
    lua_getglobal(L, field);
    if (!lua_istable(L, -1)) {
        ERROR_MSG(err, msg);
        lua_close(L);
        exit(EXIT_FAILURE);
    }
}

void lua_getField(lua_State *L, const char* field, const char* err, const char* msg) {
    lua_getfield(L, -1, field);
    if (!lua_istable(L, -1)) {
        ERROR_MSG(err, msg);
        lua_close(L);
        exit(EXIT_FAILURE);
    }
}
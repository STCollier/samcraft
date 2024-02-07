#include "util.h"

void checkOpenGLErr(const char* stmt, const char* fname, int line) {
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        printf("OpenGL error %08x, at %s:%i - for %s\n", err, fname, line, stmt);
        abort();
    }
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

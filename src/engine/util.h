#ifndef UTIL_H
#define UTIL_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <lua/lua.h>
#include <lua/lauxlib.h>
#include <lua/lualib.h>

#include "cglm/cglm.h"
#include "cglm/call.h"

#include "types.h"

#define _DEBUG true

#ifdef _DEBUG
    #define GL_CHECK(stmt) do { \
        stmt; \
            checkOpenGLErr(#stmt, __FILE__, __LINE__); \
        } while (0)
#else
    #define GL_CHECK(stmt) stmt
#endif

#define ARR_SIZE(arr) (sizeof((arr)) / sizeof((arr[0])))

void checkOpenGLErr(const char* stmt, const char* fname, int line);
uint8_t hash8(const char* h);
int idist(ivec2s a, ivec2s b);

int lua_getInt(lua_State *L, const char* field, const char* err, const char* msg);
const char* lua_getString(lua_State *L, const char* field, const char* err, const char* msg);
void lua_getField(lua_State *L, const char* field, const char* err, const char* msg);
void lua_getGlobal(lua_State *L, const char* field, const char* err, const char* msg);

// Macros
#define ERROR(x) fprintf(stderr, "\x1B[0m%s:%d: \x1B[0;31m[ERROR]\x1B[0m %s\n", __FILE__, __LINE__, x);
#define WARN(x) fprintf(stdout, "\x1B[0m%s:%d: \x1B[0;35m[WARNING]\x1B[0m %s\n", __FILE__, __LINE__, x);
#define LOG(x) fprintf(stdout, "\x1B[0m%s:%d: [LOG] %s\n", __FILE__, __LINE__, x);
#define LOG_IMSG(x, i) fprintf(stdout, "\x1B[0m%s:%d: [LOG] %s %d\n", __FILE__, __LINE__, x, i);

#define ERROR_MSG(err, msg) fprintf(stderr, "\x1B[0m%s:%d: \x1B[0;31m[ERROR]\x1B[0m %s %s\n", __FILE__, __LINE__, err, msg);
#define ERROR_IMSG(err, msg) fprintf(stderr, "\x1B[0m%s:%d: \x1B[0;31m[ERROR]\x1B[0m %s %d\n", __FILE__, __LINE__, err, msg);
#define WARN_MSG(err, msg) fprintf(stdout, "\x1B[0m%s:%d: \x1B[0;35m[WARNING]\x1B[0m %s %s\n", __FILE__, __LINE__, err, msg);
#define WARN_MSG2(err, msg, msg2) fprintf(stdout, "\x1B[0m%s:%d: \x1B[0;35m[WARNING]\x1B[0m %s %s. %s\n", __FILE__, __LINE__, err, msg, msg2);
#define LOG_MSG(err, msg) fprintf(stdout, "\x1B[0m%s:%d: [LOG] %s %s\n", __FILE__, __LINE__, err, msg);

#define STR(x) #x
#define ALMOST_EQUAL(x, y) (fabs((x) - (y)) < (0.01))
#define RAND(min, max) (rand() % (max + 1 - min) + min)

#define force_inline __attribute__((always_inline))

#endif
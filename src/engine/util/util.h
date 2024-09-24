#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <lua/lua.h>
#include <lua/lauxlib.h>
#include <lua/lualib.h>
#include <limits.h>
#include <time.h>
#include <string.h>

#include "cglm/cglm.h"
#include "cglm/call.h"

#include "types.h"

#ifndef UTIL_H
#define UTIL_H

void ivec3s_to_ivec3(ivec3s in, ivec3 out);
void ivec2s_to_ivec2(ivec2s in, ivec2 out);

#define _DEBUG

#ifdef _DEBUG
    #define GL_CHECK(stmt) do { \
        stmt; \
            checkOpenGLErr(#stmt, __FILE__, __LINE__); \
        } while (0)
#else
    #define GL_CHECK(stmt) stmt
#endif

#define EXECTIME(process, num, ...) \
    clock_t start = clock(); \
    __VA_ARGS__ \
    clock_t finish = clock(); \
    double t = (double)(finish - start) / CLOCKS_PER_SEC; \
    fprintf(stdout, "[%s %d] took %lf ms\n", process, num, t*1000); \

#define ARR_SIZE(arr) (sizeof((arr)) / sizeof((arr[0])))
#define UNUSED(x) ((void) x)

#define CONCAT_HELPER(A,B) A##B
#define CONCAT(A,B) CONCAT_HELPER(A,B)
#define STRINGIFY(A) #A

#define TAU (M_PI * 2)
#define EPSILON 0.00001
 
#define DEFINE_ARRAY_IMPL(type) \
    typedef struct { \
        size_t length, capacity; \
        type* data; \
    } CONCAT(arr_, type); \
\
    static CONCAT(arr_, type) CONCAT(type, _array)() { \
        type* data = malloc(sizeof(type)); \
        CONCAT(arr_, type) arr = {.length = 0, .capacity = 1, .data = data}; \
        return arr; \
    } \
\
    static void CONCAT(CONCAT(type, _arr), _push)(CONCAT(arr_, type)* arr, type data) { \
        if (arr->length < arr->capacity) { \
            arr->data[arr->length] = data; \
        } else { \
            arr->capacity *= 2; \
            arr->data = realloc(arr->data, arr->capacity * sizeof(type)); \
            arr->data[arr->length] = data; \
        } \
\
        arr->length++; \
    } \
\
    static type CONCAT(CONCAT(type, _arr), _pop)(CONCAT(arr_, type)* arr) { \
        return arr->data[arr->length---1]; \
    } \
\
    static void CONCAT(CONCAT(type, _arr), _delete)(CONCAT(arr_, type) *arr) { \
        arr->length = 0;\
        arr->capacity = 0;\
        free(arr->data); \
        arr->data = NULL; \
    } \
\
    static void CONCAT(CONCAT(type, _arr), _clear)(CONCAT(arr_, type) *arr) { \
        memset(arr->data, 0, arr->length); \
        arr->length = 0;\
    } \

DEFINE_ARRAY_IMPL(int);
DEFINE_ARRAY_IMPL(float);
DEFINE_ARRAY_IMPL(uint8_t);
DEFINE_ARRAY_IMPL(uint16_t);
DEFINE_ARRAY_IMPL(uint32_t);
DEFINE_ARRAY_IMPL(uint64_t);
DEFINE_ARRAY_IMPL(ivec2s);
DEFINE_ARRAY_IMPL(ivec3s);

void checkOpenGLErr(const char* stmt, const char* fname, int line);
uint8_t hash8(const char* h);
int idist2d(ivec2 a, ivec2 b);
int idist3d(ivec3 a, ivec3 b);
bool vec3_nequal(vec3 a, vec3 b);
bool ivec3_nequal(ivec3 a, ivec3 b);
int randInt(int min, int max);
int sign(float x);
float clamp(float value, float min, float max);
float dot4(vec4 a, vec4 b);
float dot3(vec3 a, vec3 b);
float packRGBA(ivec4 color);

int lua_getInt(lua_State *L, const char* field, const char* err, const char* msg);
bool lua_getBool(lua_State *L, const char* field, const char* err, const char* msg);
float lua_getFloat(lua_State *L, const char* field, const char* err, const char* msg);
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
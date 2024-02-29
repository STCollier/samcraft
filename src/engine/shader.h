#ifndef SHADER_H
#define SHADER_H

#include "cglm/cglm.h"
#include "cglm/call.h"

typedef struct {
  unsigned int ID;
} shader_t;

enum ShaderType {
  PROGRAM = 0,
  VERTEX,
  FRAGMENT
};

shader_t shader_new(const char* vertexPath, const char* fragmentPath);
void shader_use(shader_t self);

void shader_setBool(shader_t self, char* name, int value);
void shader_setInt(shader_t self, char* name, int value);
void shader_setFloat(shader_t self, char* name, float value);
void shader_setMat4(shader_t self, char* name, const mat4 mat);
void shader_setVec4(shader_t self, char* name, float x, float y, float z, float w);
void shader_setVec3(shader_t self, char* name, float x, float y, float z);
void shader_setVec2(shader_t self, char* name, float x, float y);

#endif
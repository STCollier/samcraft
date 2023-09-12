#ifndef SHADER_H
#define SHADER_H

#include "cglm/cglm.h"
#include "cglm/call.h"

struct Shader {
  unsigned int ID;
};

struct Shader createShader(const char* vertexPath, const char* fragmentPath);

void useShader(struct Shader self);
void setShaderBool(struct Shader self, char* name, int value);
void setShaderInt(struct Shader self, char* name, int value);
void setShaderFloat(struct Shader self, char* name, float value);
void setShaderMat4(struct Shader self, char* name, const mat4 mat);
void setShaderVec4(struct Shader self, char* name, float r, float g, float b);
void setShaderVec3(struct Shader self, char* name, float r, float g, float b);

#endif
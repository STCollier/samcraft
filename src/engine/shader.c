#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "cglm/cglm.h"
#include "cglm/call.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "shader.h"

static void _checkCompileErrors(unsigned int shader, enum ShaderType type) {
  int success;
  char infoLog[1024];

  if (type != PROGRAM) {
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (!success) {
      glGetShaderInfoLog(shader, 1024, NULL, infoLog);
      printf("Error compiling %s shader.\n%s\n", !type ? "program" : type ? "vertex" : "fragment", infoLog);
    }
  } else {
    glGetProgramiv(shader, GL_LINK_STATUS, &success);

    if (!success) {
      glGetProgramInfoLog(shader, 1024, NULL, infoLog);
      printf("Error linking %s shader.\n%s\n", !type ? "program" : type ? "vertex" : "fragment", infoLog);
    } 
  }
}

static char* _getFileContent(FILE* const inputFile) {
  size_t chunkSize = 1;
  size_t resultSize = 0;
  char* result = malloc(chunkSize);
  while (true) {
    const size_t bytesRead = fread(result + resultSize, sizeof(char), chunkSize, inputFile);
    if (bytesRead == chunkSize) {
      resultSize += chunkSize;
      chunkSize *= 2;
      result = realloc(result, resultSize + chunkSize);
    } else {
      result[resultSize + bytesRead] = '\0';
      break;
    }
  }

  return result;
}

shader_t shader_new(const char* vertexPath, const char* fragmentPath) {
  shader_t self;

  FILE *fShaderFile = fopen(fragmentPath, "rb");
  FILE *vShaderFile = fopen(vertexPath, "rb");

  char* vertexShaderCode = _getFileContent(vShaderFile);
  char* fragmentShaderCode = _getFileContent(fShaderFile);
  unsigned int vertexShader, fragmentShader;

  //Vertex shader
  vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, (const char* const*) &vertexShaderCode, NULL);
  glCompileShader(vertexShader);
  _checkCompileErrors(vertexShader, VERTEX);

  //Fragment shader
  fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, (const char* const*) &fragmentShaderCode, NULL);
  glCompileShader(fragmentShader);  
  _checkCompileErrors(fragmentShader, FRAGMENT);

  //Shader program
  self.ID = glCreateProgram();
  glAttachShader(self.ID, vertexShader);
  glAttachShader(self.ID, fragmentShader);
  glLinkProgram(self.ID);
  _checkCompileErrors(self.ID, PROGRAM);

  //Delete shaders
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  free(vertexShaderCode);
  free(fragmentShaderCode);
  fclose(fShaderFile);
  fclose(vShaderFile);
  
  return self;
}

void shader_use(shader_t self) {
  glUseProgram(self.ID);
}

void shader_setBool(shader_t self, char* name, int value) {
  glUniform1i(glGetUniformLocation(self.ID, name), (int) value);
}

void shader_setInt(shader_t self, char* name, int value) {
  glUniform1i(glGetUniformLocation(self.ID, name), value);
}

void shader_setFloat(shader_t self, char* name, float value) {
  glUniform1f(glGetUniformLocation(self.ID, name), value);
}

void shader_setMat4(shader_t self, char* name, const mat4 mat) {
  glUniformMatrix4fv(glGetUniformLocation(self.ID, name), 1, GL_FALSE, &mat[0][0]);
}

void shader_setVec4(shader_t self, char* name, float r, float g, float b) {
  glUniform4f(glGetUniformLocation(self.ID, name), r, g, b, 1.0f);
}

void shader_setVec3(shader_t self, char* name, float r, float g, float b) { 
  glUniform3f(glGetUniformLocation(self.ID, name), r, g, b); 
}
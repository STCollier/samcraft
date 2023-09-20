#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include "cglm/cglm.h"
#include "cglm/call.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "shader.h"
#include "util.h"

static void _checkCompileErrors(unsigned int shader, char* type) {
  int success;
  char infoLog[1024];

  if (strcmp(type, "PROGRAM") != 0) {
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (!success) {
      glGetShaderInfoLog(shader, 1024, NULL, infoLog);
      printf("Error compiling %s shader.\n%s\n", type, infoLog);
    }
  } else {
    glGetProgramiv(shader, GL_LINK_STATUS, &success);

    if (!success) {
      glGetProgramInfoLog(shader, 1024, NULL, infoLog);
      printf("Error linking %s shader.\n%s\n", type, infoLog);
    } 
  }
}

static char* _getFileContent(const char* filename) {
  char* buffer = 0;
  long length;
  FILE* file = fopen (filename, "rb");

  if (file) {
    fseek(file, 0, SEEK_END);
    length = ftell(file);
    fseek(file, 0, SEEK_SET);
    buffer = malloc(length + 1);

    if (buffer) {
      fread(buffer, 1, length, file);
      buffer[length] = '\0'; // Null terminate string
    }

    fclose(file);
  } else {
    ERROR_MSG("Error finding shader file: ", filename);
    exit(1);
  }

  return buffer;
}


struct Shader createShader(const char* vertexPath, const char* fragmentPath) {
  struct Shader self;

  const char* vertexShaderCode = _getFileContent(vertexPath);
  const char* fragmentShaderCode = _getFileContent(fragmentPath);
  unsigned int vertexShader, fragmentShader;

  //Vertex shader
  vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vertexShaderCode, NULL);
  glCompileShader(vertexShader);
  _checkCompileErrors(vertexShader, "VERTEX");

  //Fragment shader
  fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fragmentShaderCode, NULL);
  glCompileShader(fragmentShader);  
  _checkCompileErrors(fragmentShader, "FRAGMENT");

  //Shader program
  self.ID = glCreateProgram();
  glAttachShader(self.ID, vertexShader);
  glAttachShader(self.ID, fragmentShader);
  glLinkProgram(self.ID);
  _checkCompileErrors(self.ID, "PROGRAM");

  LOG("Shader compiled successfully!");

  //Delete shaders
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
  
  return self;
}

void useShader(struct Shader self) {
  glUseProgram(self.ID);
}

void setShaderBool(struct Shader self, char* name, int value) {
  glUniform1i(glGetUniformLocation(self.ID, name), (int) value);
}

void setShaderInt(struct Shader self, char* name, int value) {
  glUniform1i(glGetUniformLocation(self.ID, name), value);
}

void setShaderFloat(struct Shader self, char* name, float value) {
  glUniform1f(glGetUniformLocation(self.ID, name), value);
}

void setShaderMat4(struct Shader self, char* name, const mat4 mat) {
  glUniformMatrix4fv(glGetUniformLocation(self.ID, name), 1, GL_FALSE, &mat[0][0]);
}

void setShaderVec4(struct Shader self, char* name, float r, float g, float b) {
  glUniform4f(glGetUniformLocation(self.ID, name), r, g, b, 1.0f);
}

void setShaderVec3(struct Shader self, char* name, float r, float g, float b) { 
  glUniform3f(glGetUniformLocation(self.ID, name), r, g, b); 
}
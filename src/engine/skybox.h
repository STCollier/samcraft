#ifndef SKYBOX_H
#define SKYBOX_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "cglm/cglm.h"
#include "cglm/call.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "shader.h"
#include "camera.h"
#include "window.h"

struct Skybox {
	unsigned int VBO, VAO;
};

struct Skybox skybox_new();
void skybox_render(struct Skybox *skybox, shader_t shader);

#endif
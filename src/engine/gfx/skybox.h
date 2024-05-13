#ifndef SKYBOX_H
#define SKYBOX_H

#include "../util/common.h"

struct Skybox {
	unsigned int VBO, VAO;
};

struct Skybox skybox_new();
void skybox_render(struct Skybox *skybox, shader_t shader);

#endif